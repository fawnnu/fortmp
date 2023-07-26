#include "include.hpp"
#include "vehicles_component.hpp"

#include "inventory_component.hpp"

vehicles_component::vehicles_component()
{
    INIT_POINTER(SetWorldLocationAndRotation, memory_manager::find_pattern("40 55 53 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 C7 41 0F 10 00 4C 8B 7D 77 48 8B D9 48 8B 89 ? ? ? ? 45 0F B6 F1 0F 29 45 87 49 8B F8"));
    INIT_POINTER(SetWorldLocationAndRotation, memory_manager::find_pattern("40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 41 0F 10 00 4C 8B B4 24 ? ? ? ?"));
    
    // Hook the last call in EquipAbilityWeapon
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortGameplayAbility.EquipAbilityWeapon", vehicles_component_hooks::EquipAbilityWeapon_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerRequestSeatChange, L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange", vehicles_component_hooks::ServerRequestSeatChange_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerFireActorInCannon, L"/Script/FortniteGame.FortWeaponRangedMountedCannon.ServerFireActorInCannon", vehicles_component_hooks::ServerFireActorInCannon_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerOnExitVehicle, L"/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle", vehicles_component_hooks::ServerOnExitVehicle_Hook);
    
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
    if (target_player_controller->WorldInventory())
    {
        TArrayManager ReplicatedEntries = TArrayManager<FFortItemEntry>(&target_player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

        for (int i = ReplicatedEntries.Num() - 1; i >= 0; i--)
        {
            FFortItemEntry& ReplicatedEntry = ReplicatedEntries[i];
            
            if (UFortItemDefinition* ItemDefinition = ReplicatedEntry.ItemDefinition())
            {
                if (ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()))
                {
                    if (UObject* WeaponActorClass = static_cast<UFortWeaponItemDefinition*>(ItemDefinition)->WeaponActorClass().Get())
                    {
                        if (WeaponActorClass->This_IsA(AFortWeaponRangedForVehicle::StaticClass()) ||
                            WeaponActorClass->This_IsA(AFortWeaponRangedDualForVehicle::StaticClass()))
                        {
                            inventory_component::remove_item(target_player_controller, ReplicatedEntry.ItemGuid(), -1, false);
                        }
                    }
                }
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

void vehicles_component_hooks::ServerOnExitVehicle_Hook(AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame)
{
    ServerOnExitVehicle(FortPlayerPawn, StackFrame);

    if (FortPlayerPawn && FortPlayerPawn->Controller())
    {
        AFortPlayerController* FortPlayerController = FortPlayerPawn->Controller();
    
        // Unequip the equipped vehicle weapon
        vehicles_component::unequip_vehicle_weapon(FortPlayerController);

        if (UFortWorldItem* Pickaxe = inventory_component::get_pickaxe(FortPlayerController))
        {
            FortPlayerController->ServerExecuteInventoryItem(Pickaxe->GetItemGuid());
        }
    }
}

void vehicles_component_hooks::ServerRequestSeatChange_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{

    // Unequip vehicle weapon
    vehicles_component::unequip_vehicle_weapon(FortPlayerController);

    if (UFortVehicleSeatWeaponComponent::StaticClass())
    {
        GET_PARAMETER(int, TargetSeatIndex);
        
        if (FortPlayerController->Pawn()->IsInVehicle())
        {
            if (UFortVehicleSeatWeaponComponent* SeatWeaponComponent = FortPlayerController->Pawn()->GetVehicle()->GetSeatWeaponComponent(TargetSeatIndex))
            {
                UFortWeaponItemDefinition* VehicleWeapon = nullptr;
                TArrayManager WeaponSeatDefinitions = TArrayManager<FWeaponSeatDefinition>(&SeatWeaponComponent->WeaponSeatDefinitions(), FWeaponSeatDefinition::GetStructSize());
                
                for (FWeaponSeatDefinition& SeatDefinition : WeaponSeatDefinitions)
                {
                    if (SeatDefinition.SeatIndex() == TargetSeatIndex)
                    {
                        VehicleWeapon = SeatDefinition.VehicleWeapon();
                    }
                }
                
                vehicles_component::equip_vehicle_weapon(FortPlayerController, VehicleWeapon);
                
                return ServerRequestSeatChange(FortPlayerController, StackFrame);
            }
        }
    }
    
    // First entry is always pickaxe
    auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
    
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

void vehicles_component_hooks::ServerFireActorInCannon_Hook(AFortWeaponRangedForVehicle* Weapon, FFrame& StackFrame)
{
    GET_PARAMETER(FVector, LaunchDir);

    // Get instigator
    AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(Weapon->Instigator());

    if (Pawn)
    {
        if (Pawn->IsInVehicle())
        {
            // Get vehicle
            if (AFortAthenaVehicle* Vehicle = Pawn->GetVehicle())
            {
                // Get pawn at seat 1 (inside the cannon) and shoot it
                AFortPlayerPawn* PawnToShoot = Vehicle->GetPawnAtSeat(1);

                if (PawnToShoot)
                {
                    if (Vehicle->IsA(AFortAthenaSKPushCannon::StaticClass()))
                    {
                        AFortAthenaSKPushCannon* PushCannon = static_cast<AFortAthenaSKPushCannon*>(Vehicle);

                        PushCannon->OnPreLaunchPawn(PawnToShoot, LaunchDir);
                        PawnToShoot->ServerOnExitVehicle();
                        PushCannon->OnLaunchPawn(PawnToShoot, LaunchDir);
                        PushCannon->MultiCastPushCannonLaunchedPlayer();
                    }
                }
            }
        }
    }

    return ServerFireActorInCannon(Weapon, StackFrame);
}