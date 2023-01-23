#include "include.hpp"
#include "vehicles_component.hpp"

#include "inventory_component.hpp"

vehicles_component::vehicles_component()
{
    INIT_POINTER(SetWorldLocationAndRotation, memory_manager::find_pattern("40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 41 0F 10 00 4C 8B B4 24 ? ? ? ?"));
    
    // Hook the last call in EquipAbilityWeapon
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortGameplayAbility.EquipAbilityWeapon", vehicles_component_hooks::EquipAbilityWeapon_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerAttemptExitVehicle, L"/Script/FortniteGame.FortPlayerControllerZone.ServerAttemptExitVehicle", vehicles_component_hooks::ServerAttemptExitVehicle_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerRequestSeatChange, L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange", vehicles_component_hooks::ServerRequestSeatChange_Hook);

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortAthenaVehicle.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerMove", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    
    DetourTransactionCommit();
}

void vehicles_component::equip_vehicle_weapon(AFortPlayerController* target_player_controller, UFortWeaponItemDefinition* weapon_item_definition)
{
    // Make sure input player is valid
    if (target_player_controller)
    {
        // make sure the weapon definition is valid
        if (weapon_item_definition)
        {
            if (inventory_component::get_inventory_item_by_definition(target_player_controller, weapon_item_definition) == nullptr)
            {
                // add it to the inventory
                auto new_item = inventory_component::add_item(target_player_controller, weapon_item_definition, 1, INT_MAX);

                // equip the ability weapon
                auto CurrentWeapon = target_player_controller->Pawn()->EquipWeaponDefinition(weapon_item_definition, new_item->ItemEntry().ItemGuid());

                // Set more in currentweapon
                if (CurrentWeapon->IsA(AFortWeaponRangedForVehicle::StaticClass()))
                {
                    if (STATIC_HASPROPERTY(CurrentWeapon, MountedWeaponInfoRepped))
                    {
                        static_cast<AFortWeaponRangedForVehicle*>(CurrentWeapon)->MountedWeaponInfoRepped().HostVehicleCachedActor() = target_player_controller->Pawn()->GetVehicle();
                        static_cast<AFortWeaponRangedForVehicle*>(CurrentWeapon)->MountedWeaponInfoRepped().HostVehicleSeatIndexCached() = target_player_controller->Pawn()->GetVehicleSeatIndex();

                        static_cast<AFortWeaponRangedForVehicle*>(CurrentWeapon)->OnHostVehicleSetup();
                        static_cast<AFortWeaponRangedForVehicle*>(CurrentWeapon)->OnRep_MountedWeaponInfoRepped();
                    }
                }
                else if (CurrentWeapon->IsA(AFortWeaponRangedDualForVehicle::StaticClass()))
                {
                    if (STATIC_HASPROPERTY(CurrentWeapon, MountedWeaponInfoRepped))
                    {
                        static_cast<AFortWeaponRangedDualForVehicle*>(CurrentWeapon)->MountedWeaponInfoRepped().HostVehicleCachedActor() = target_player_controller->Pawn()->GetVehicle();
                        static_cast<AFortWeaponRangedDualForVehicle*>(CurrentWeapon)->MountedWeaponInfoRepped().HostVehicleSeatIndexCached() = target_player_controller->Pawn()->GetVehicleSeatIndex();

                        static_cast<AFortWeaponRangedDualForVehicle*>(CurrentWeapon)->OnHostVehicleSetup();
                        static_cast<AFortWeaponRangedDualForVehicle*>(CurrentWeapon)->OnRep_MountedWeaponInfoRepped();   
                    }
                }
            }
        }
    }
}

void vehicles_component::unequip_vehicle_weapon(AFortPlayerController* target_player_controller)
{
    if (target_player_controller->MyFortPawn())
    {
        if (AFortWeapon* CurrentWeapon = target_player_controller->MyFortPawn()->CurrentWeapon())
        {
            if (CurrentWeapon->IsA(AFortWeaponRangedForVehicle::StaticClass()) || CurrentWeapon->IsA(AFortWeaponRangedDualForVehicle::StaticClass()))
            {
                // Remove vehicle weapon
                inventory_component::remove_item(target_player_controller, CurrentWeapon->ItemEntryGuid(), -1, false, false);
            }
        }
    }
}

// re-add code for this function; it's stripped
void vehicles_component_hooks::EquipAbilityWeapon_Hook(UFortGameplayAbility* FortGameplayAbility, UFortWeaponItemDefinition* WeaponDefinition)
{
    if (FortGameplayAbility->GetActivatingPawn())
    {
        vehicles_component::equip_vehicle_weapon(FortGameplayAbility->GetActivatingPawn()->Controller(), WeaponDefinition);
    }
}

void vehicles_component_hooks::ServerAttemptExitVehicle_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    // Unequip the equipped vehicle weapon
    vehicles_component::unequip_vehicle_weapon(FortPlayerController);
    
    // First entry is always pickaxe
    auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
        
    return ServerAttemptExitVehicle(FortPlayerController, StackFrame);
}

void vehicles_component_hooks::ServerRequestSeatChange_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(int, TargetSeatIndex);

    // Switching to driver seat, equip vehicle weapon on newer versions
    if (TargetSeatIndex == 0)
    {
        if (UFortVehicleSeatWeaponComponent::StaticClass())
        {
            if (FortPlayerController->Pawn()->IsInVehicle())
            {
                unsigned char SeatIndex = FortPlayerController->Pawn()->GetVehicleSeatIndex();

                if (UFortVehicleSeatWeaponComponent* SeatWeaponComponent = FortPlayerController->Pawn()->GetVehicle()->GetSeatWeaponComponent(SeatIndex))
                {
                    UFortWeaponItemDefinition* VehicleWeapon = nullptr;
                    TArrayManager WeaponSeatDefinitions = TArrayManager<FWeaponSeatDefinition>(&SeatWeaponComponent->WeaponSeatDefinitions(), FWeaponSeatDefinition::GetStructSize());
                
                    for (FWeaponSeatDefinition& SeatDefinition : WeaponSeatDefinitions)
                    {
                        if (SeatDefinition.SeatIndex() == SeatIndex)
                        {
                            VehicleWeapon = SeatDefinition.VehicleWeapon();
                        }
                    }
                
                    vehicles_component::equip_vehicle_weapon(FortPlayerController, VehicleWeapon);
                }
            }
        }
    }
    else // Remove vehicle weapon on other seats
    {
        vehicles_component::unequip_vehicle_weapon(FortPlayerController);
    
        // Equip pickaxe
        auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
        FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
    }
    
    return ServerRequestSeatChange(FortPlayerController, StackFrame);   
}

// re-add vehicle sync
void vehicles_component_hooks::ServerUpdatePhysicsParams_Hook(AActor* Vehicle, FFrame& StackFrame)
{
    GET_PARAMETER(FReplicatedPhysicsPawnState, InState);

    if (const auto RootComponent = Vehicle->RootComponent())
    {
        if (SetWorldLocationAndRotation)
        {
            // Credits: wiktorwiktor12
            FQuat FixedRot;
            FixedRot.X = InState.Rotation.X + -2.5;
            FixedRot.Y = 3.333333 * InState.Rotation.Y + 3.18982 * pow(10,-7);
            FixedRot.Z = InState.Rotation.Z + 2;
            FixedRot.W = 8.26471 * pow(10,-6) - 0.833319 * InState.Rotation.W;
            
            SetWorldLocationAndRotation(RootComponent, InState.Translation, FixedRot, false, nullptr, 1);
        }
        else
        {
            spdlog::error(L"vehicles_component_hooks::ServerUpdatePhysicsParams_Hook: SetWorldLocationAndRotation is NULL.");
        }

        RootComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, L"None");
        
        RootComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, false, L"None");
    }
    
    return ServerUpdatePhysicsParams(Vehicle, StackFrame);   
}
