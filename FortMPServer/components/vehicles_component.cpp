#include "include.hpp"
#include "vehicles_component.hpp"

#include "challenges_component.hpp"
#include "inventory_component.hpp"
#include "loader_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"

vehicles_component::vehicles_component()
{
    if (AFortAthenaVehicleSpawner::StaticClass())
    {
        AFortPlayerController* Default__FortPlayerController = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerController");

        if (const uintptr_t BeginPlayAddress = memory_manager::find_function_by_string(L"MusicManager class default object is null as FortPlayerController::BeginPlay is called"))
        {
            int BeginPlayIndex = 0;
    
            for (int i = 0; i < INT_MAX; i++)
            {
                if (Default__FortPlayerController->VTable[i] == reinterpret_cast<LPVOID>(BeginPlayAddress))
                {
                    BeginPlayIndex = i;
            
                    break;
                }
            }

            // Hook begin play for the classes we need
            REGISTER_VTABLE_HOOK(CreateDefaultObject<AFortAthenaVehicleSpawner>()->VTable, BeginPlayIndex, vehicles_component_hooks::AFortAthenaVehicleSpawner_BeginPlay);
        }
        else
        {
            spdlog::error(L"Failed to hook AFortAthenaVehicleSpawner::BeginPlay. Invalid BeginPlayAddress");
        }
    }
    
    INIT_POINTER(SetWorldLocationAndRotation, memory_manager::find_pattern("40 55 53 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 C7 41 0F 10 00 4C 8B 7D 77 48 8B D9 48 8B 89 ? ? ? ? 45 0F B6 F1 0F 29 45 87 49 8B F8"));
    INIT_POINTER(SetWorldLocationAndRotation, memory_manager::find_pattern("40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 41 0F 10 00 4C 8B B4 24 ? ? ? ?"));
    INIT_POINTER(FBodyInstance_GetUnrealWorldTransform, memory_manager::find_pattern("4C 8B DC 45 88 4B 20 45 88 43 18 53 48 83 EC 50 0F 28 05 ? ? ? ? 49 8D 43 18 49 89 43 E8 48 8B DA 0F 11 02 49 8D 43 20 0F 28 0D ? ? ? ? 49 89 43 F0 49 8D 43 D8 0F 11 4A 10 0F 28 05 ? ? ? ? "));
    
    // Hook the last call in EquipAbilityWeapon
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortGameplayAbility.EquipAbilityWeapon", vehicles_component_hooks::EquipAbilityWeapon_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(vehicles_component_hooks::OnDamaged, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 41 54 41 56 48 81 EC ? ? ? ? 48 8B 01 49 8B D9 44 0F 29 44 24 ? 4D 8B E0 44 0F 28 C1 48 8B F9 FF 90 ? ? ? ? 48 8B 83 ? ? ? ? 48 8D 35 ? ? ? ? 48 89 44 24 ? 48 8B 83 ? ? ? ? 48 89 74 24 ? 48 89 44 24 ? 48 85 C0 74 03 FF 40 08 48 8B 84 24 ? ? ? ? 4C 8D 4C 24 ? 4C 8B B4 24 ? ? ? ? 4D 8B C4"), vehicles_component_hooks::OnDamaged_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerRequestSeatChange, L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange", vehicles_component_hooks::ServerRequestSeatChange_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerFireActorInCannon, L"/Script/FortniteGame.FortWeaponRangedMountedCannon.ServerFireActorInCannon", vehicles_component_hooks::ServerFireActorInCannon_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerOnExitVehicle, L"/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle", vehicles_component_hooks::ServerOnExitVehicle_Hook);
    
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortAthenaVehicle.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerMove", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);

    // Ballers universal hook CR. wiktorwiktor12
    auto FortAttachableProjectileUpdateState = memory_manager::find_function_by_string(L"&AFortAttachableProjectileBase::OnAttachedToEndPlayed");

    if (FortAttachableProjectileUpdateState)
    {
        static auto dFortAttachableProjectileBase = FindObject(L"/Script/FortniteGame.Default__FortAttachableProjectileBase");
        static auto dFortOctopusTowhookAttachableProjectile = FindObject(L"/Script/FortniteGame.Default__FortOctopusTowhookAttachableProjectile");
        
        FortAttachableProjectileUpdateState = memory_manager::get_function_start(FortAttachableProjectileUpdateState);
        
        int UpdateStateIndex = 0;
    
        for (int i = 0; i < 9999; i++)
        {
            if (dFortAttachableProjectileBase->VTable[i] == reinterpret_cast<LPVOID>(FortAttachableProjectileUpdateState))
            {
                UpdateStateIndex = i * 8;
        
                break;
            }
        }
        OctopusTowhookUpdateState = *(void**)(__int64(dFortOctopusTowhookAttachableProjectile->VTable) + UpdateStateIndex);
        auto strippedCalls = memory_manager::get_all_stripped_calls_in_function_wiktor(memory_manager::get_function_start(reinterpret_cast<uintptr_t>(OctopusTowhookUpdateState)));
        if (strippedCalls.size() > 0)
        {
            auto UpdateAttachState = strippedCalls[0];

            if (UpdateAttachState)
            {
                REGISTER_CALL_DETOUR_WITH_ADDRESS(UpdateAttachState, vehicles_component_hooks::UpdateAttachState_Hook);   
            }
        }
        
    }
    DetourTransactionCommit();
}

void vehicles_component_hooks::UpdateAttachState_Hook(AFortOctopusVehicle* OctopusVehicle, TWeakObjectPtr<UPrimitiveComponent>& WeakComponent, FVector& WorldLocation, FVector& WorldNormal)
{
    // Credit wiktorwiktor12 and ruby
    UPrimitiveComponent* Component = WeakComponent.Get();
    if (Component)
    {
        typedef FBodyInstance* (__fastcall *GetBodyInstanceType)(UPrimitiveComponent*, FName, __int64);
        
        void* result = memory_manager::get_virtual_function(Component, 0x668);
        GetBodyInstanceType GetBodyInstance = reinterpret_cast<GetBodyInstanceType>(result);

        FBodyInstance* BodyInstance = GetBodyInstance(Component, FName(L""), true);

        if (BodyInstance)
        {
            FTransform WorldTransform{};
            FBodyInstance_GetUnrealWorldTransform(BodyInstance, &WorldTransform, true, false);
            
            OctopusVehicle->ReplicatedAttachState().Component() = Component;
            OctopusVehicle->ReplicatedAttachState().LocalLocation()  = UKismetMathLibrary::InverseTransformLocation(WorldTransform, WorldLocation);

            OctopusVehicle->OnRep_ReplicatedAttachState();
        }
    }
}

void vehicles_component_hooks::AFortAthenaVehicleSpawner_BeginPlay_Hook(AFortAthenaVehicleSpawner* Target)
{
    AFortAthenaVehicleSpawner_BeginPlay(Target);

    if (Target->HasProperty(L"SpawnedVehicle") && Target->SpawnedVehicle() == nullptr) return;
    
    UObject* vehicle_class = Target->GetVehicleClass();
    if (!vehicle_class) return;
    
    auto GameState = (*GWorld)->GameState();
    if (!GameState) return;

    auto MapInfo = GameState->MapInfo();
    
    if (MapInfo)
    {
        if (STATIC_HASPROPERTY(MapInfo, VehicleClassDetails))
        {
            for (int i = 0; i < MapInfo->VehicleClassDetails().Count; ++i)
            {
                TArrayManager VehicleClassDetails = MapInfo->VehicleClassDetails();
                FVehicleClassDetails& Detail = VehicleClassDetails[i];

                if (Detail.VehicleClass().Get() != vehicle_class) continue;

                float min_percent = Detail.VehicleMinSpawnPercent().GetValueAtLevel(0, nullptr);
                float max_percent = Detail.VehicleMaxSpawnPercent().GetValueAtLevel(0, nullptr);

                float spawn_percent = UKismetMathLibrary::RandomFloatInRange(min_percent, std::max(max_percent, min_percent));

                float random_percent = UKismetMathLibrary::RandomFloatInRange(0, 100);

                if (random_percent <= spawn_percent)
                {
                    Target->SpawnVehicle();
                }
            } 
        }
    }
    
    if (STATIC_HASPROPERTY(Target, FortVehicleItemDef))
    {
        auto vehicle_def = Target->FortVehicleItemDef().Get();

        if (vehicle_def->VehicleActorClass().Get() != vehicle_class) return;

        if (!vehicle_def->HasProperty(L"VehicleMinSpawnPercent") && !vehicle_def->HasProperty(L"VehicleMaxSpawnPercent")) return;
        
        float min_percent = vehicle_def->VehicleMinSpawnPercent().GetValueAtLevel(0, nullptr);
        float max_percent = vehicle_def->VehicleMaxSpawnPercent().GetValueAtLevel(0, nullptr);

        float spawn_percent = UKismetMathLibrary::RandomFloatInRange(min_percent, std::max(max_percent, min_percent));

        float random_percent = UKismetMathLibrary::RandomFloatInRange(0, 100);

        if (random_percent <= spawn_percent)
        {
            Target->SpawnVehicle();
        }
    }
    
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
    
    // Equip Pickaxe
    if (auto pickaxe = inventory_component::get_pickaxe(FortPlayerController))
    {
        FortPlayerController->ServerExecuteInventoryItem(pickaxe->ItemEntry().ItemGuid());   
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

void vehicles_component_hooks::ServerFireActorInCannon_Hook(AFortWeaponRangedForVehicle* Weapon, FFrame& StackFrame)
{
    ServerFireActorInCannon(Weapon, StackFrame);
    
    GET_PARAMETER(FVector, LaunchDir);
    
    if (Weapon == nullptr || memory_manager::is_bad_read_pointer(Weapon, false)) return;
    if (Weapon->IsA(AFortWeaponRangedForVehicle::StaticClass()) == false) return;
    if (Weapon->Owner() == nullptr || memory_manager::is_bad_read_pointer(Weapon->Owner(), false)) return;
    
    if (Weapon->Owner()->IsA(AFortPlayerPawn::StaticClass()))
    {
        // Get instigator
       AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(Weapon->Owner());

       if (Pawn && !memory_manager::is_bad_read_pointer(Pawn, false))
       {
           if (Pawn->IsInVehicle())
           {
               // Get vehicle
               if (AFortAthenaVehicle* Vehicle = Pawn->GetVehicle())
               {
                   if (memory_manager::is_bad_read_pointer(Vehicle, false)) return;
                
                   // Get pawn at seat 1 (inside the cannon) and shoot it
                   AFortPlayerPawn* PawnToShoot = Vehicle->GetPawnAtSeat(1);

                   if (PawnToShoot && !memory_manager::is_bad_read_pointer(PawnToShoot, false))
                   {
                       spdlog::info(L"Vehicle {}", Vehicle->GetName().c_str());
                       if (Vehicle->IsA(AFortAthenaSKPushCannon::StaticClass()))
                       {
                           AFortAthenaSKPushCannon* PushCannon = static_cast<AFortAthenaSKPushCannon*>(Vehicle);

                           if (PushCannon && !memory_manager::is_bad_read_pointer(PushCannon, false))
                           {
                               if (LaunchDir.IsValid())
                               {
                                   spdlog::info(L"PushCannon {}, launching player", PushCannon->GetName().c_str());
                                   
                                   static UFunction* Function_OnPreLaunchPawn = FindObject<UFunction>(L"/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C.OnPreLaunchPawn");

                                   if (Function_OnPreLaunchPawn)
                                   {
                                       struct
                                       {
                                           AFortPlayerPawn* Pawn;
                                           FVector LaunchDir;
                                       } params;

                                       params.Pawn = Pawn;
                                       params.LaunchDir = LaunchDir;

                                       ProcessEvent(PushCannon, Function_OnPreLaunchPawn, &params);   
                                   }

                                   static UFunction* Function_ServerOnExitVehicle = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle");

                                   if (Function_ServerOnExitVehicle)
                                   {
                                       ProcessEvent(PushCannon, Function_ServerOnExitVehicle, nullptr);   
                                   }
                                   
                                   static UFunction* Function_OnLaunchPawn = FindObject<UFunction>(L"/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C.OnLaunchPawn");

                                   if (Function_OnLaunchPawn)
                                   {
                                       struct
                                       {
                                           AFortPlayerPawn* Pawn;
                                           FVector LaunchDir;
                                       } params;

                                       params.Pawn = Pawn;
                                       params.LaunchDir = LaunchDir;

                                       ProcessEvent(PushCannon, Function_OnLaunchPawn, &params);   
                                   }

                                   static UFunction* Function_MultiCastPushCannonLaunchedPlayer = FindObject<UFunction>(L"/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C.MultiCastPushCannonLaunchedPlayer");

                                   if (Function_MultiCastPushCannonLaunchedPlayer)
                                   {
                                       ProcessEvent(PushCannon, Function_MultiCastPushCannonLaunchedPlayer, nullptr);   
                                   }
                               }
                           }
                       }
                   }
               }
           }
       }   
    }
}

void vehicles_component_hooks::OnDamaged_Hook(AFortAthenaVehicle* Vehicle, float Damage, FGameplayTagContainer& DamageTags, FGameplayEffectContextHandle& EffectContext, class AController* EventInstigator, class AActor* DamageCauser)
{
    OnDamaged(Vehicle, Damage,  DamageTags, EffectContext, EventInstigator, DamageCauser);

    if (auto driver = Vehicle->GetDriver())
    {
        AFortPlayerControllerAthena* DamageInstigator = EventInstigator->Cast<AFortPlayerControllerAthena>();
        
        if (player_component::is_live_spectator(DamageInstigator))
        {
            return;
        }
        
        if (driver->PlayerState()->TeamIndex() != DamageInstigator->PlayerState()->TeamIndex())
        {
            DamageInstigator->EraPlayerInfo().DamageDealtTracker.EnemyVehicleDamage += Damage;
            if (UKismetSystemLibrary::K2_IsTimerActive(DamageInstigator, L"EnemyVehicleDamageCallback"))
            {
                UKismetSystemLibrary::K2_ClearTimer(DamageInstigator, L"EnemyVehicleDamageCallback");
            }
            UKismetSystemLibrary::K2_SetTimer(DamageInstigator, L"EnemyVehicleDamageCallback", 1.0, false);

            if (log_component::enable_debug_logs)
            {
                spdlog::warn(L"AFortAthenaVehicle::OnDamaged: Vehicle driven by an opponent has received damaged from {}, damage dealt is {}.", EventInstigator->Cast<AFortPlayerController>()->MyPlayerInfo()->PlayerName().c_str(), Damage);   
            }
        }
    }
}