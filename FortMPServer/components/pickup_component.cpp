#include "include.hpp"
#include "pickup_component.hpp"

#include "function_component.hpp"
#include "inventory_component.hpp"
#include "looting_component.hpp"
#include "replication_component.hpp"

pickup_component::pickup_component()
{
    INIT_POINTER(InternalHandlePickup, memory_manager::find_function_by_string(L"Attempted to spawn non-world item %s!"));
    if (const uintptr_t TargetAddress = memory_manager::find_pattern("FF 93 ? ? ? ? 49 63 8F ? ? ? ? 85 C9 0F 84 ? ? ? ? 49 8B 87 ? ? ? ? 48 89 AC 24 ? ? ? ? 48 89 B4 24 ? ? ? ? 48 8B F1"))
    {
        INIT_POINTER(InternalHandlePickup, FindObject(L"/Script/FortniteGame.Default__FortPickupAthena")->VTable[*reinterpret_cast<int*>(TargetAddress + 2) / 8] );
    }
    
    INIT_POINTER(Toss, memory_manager::get_function_start(memory_manager::find_pattern("E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B CB 48 8B 5C 24 ? 48 83 C4 20 5F")));

    //TODO: Universal sig
    INIT_POINTER(InitializePickup, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 41 56 48 83 EC 20 80 B9 ? ? ? ? ? 45 0F B6 F1"));

    // Retrieve CombinePickup's address at first, and then call detour CombinePickupFinished
    if (UFunction* OnRep_PickupLocationData = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_PickupLocationData"))
    {
        if (void* NativeFunction = OnRep_PickupLocationData->get_native_function())
        {
            if (uintptr_t scan_result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(NativeFunction), {0x48, 0xff, 0xa0}))
            {
                const int function_index = *reinterpret_cast<int*>(scan_result + 3) / 8;

                const uintptr_t OnRep_PickupLocationData_Address = reinterpret_cast<uintptr_t>(CreateDefaultObject<AFortPickupAthena>()->VTable[function_index]);

                if (uintptr_t lea_hook_address = memory_manager::find_pattern("48 8D 05 ? ? ? ? 45 33 FF 48 89 44 24 ? 48 8B CB 44 89 BB ? ? ? ? 4C 89 7C 24 ?"))
                {
                    // Get the starting address
                    INIT_POINTER(InternalCombinePickup, memory_manager::get_function_start(lea_hook_address));

                    // Lea hook
                    memory_manager::lea_hook(lea_hook_address, reinterpret_cast<uintptr_t>(pickup_component_hooks::CombinePickupFinished_Hook));
                }
                else
                {
                    scan_result = memory_manager::scan_for(OnRep_PickupLocationData_Address, {0x74, WILDCARD, 0x48, WILDCARD, WILDCARD, WILDCARD, WILDCARD, WILDCARD, WILDCARD, WILDCARD, 0xE9});

                    if (scan_result)
                    {
                        scan_result += 10;
                        const uintptr_t CombinePickupAddress = scan_result + 5 + *reinterpret_cast<int*>(scan_result + 1);
                    
                        INIT_POINTER(InternalCombinePickup, CombinePickupAddress);

                        if (CombinePickupAddress)
                        {
                            scan_result = memory_manager::scan_for(CombinePickupAddress, {0x48, 0x8d, 0x5});

                            if (scan_result)
                            {
                                memory_manager::lea_hook(scan_result, reinterpret_cast<uintptr_t>(pickup_component_hooks::CombinePickupFinished_Hook));
                            }
                        }
                    }
                }
            }
        }
    }
    
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorld", pickup_component_hooks::K2_SpawnPickupInWorld_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(pickup_component_hooks::OnAboutToEnterBackpack, memory_manager::get_function_start(memory_manager::find_pattern("74 48 48 8D 8B ? ? ? ? E8 ? ? ? ? 48 8B 93 ? ? ? ? 48 8B C8")), pickup_component_hooks::OnAboutToEnterBackpack_Hook);
    REGISTER_FUNCTION_DETOUR(pickup_component_hooks::OnAboutToEnterBackpack, memory_manager::get_function_start(memory_manager::find_pattern("E8 ? ? ? ? 48 8B 96 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8D 8E ? ? ? ? E8 ? ? ? ?")), pickup_component_hooks::OnAboutToEnterBackpack_Hook);
    REGISTER_UFUNCTION_DETOUR(pickup_component_hooks::ServerHandlePickup, L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup", pickup_component_hooks::ServerHandlePickup_Hook);
    REGISTER_UFUNCTION_DETOUR(pickup_component_hooks::ServerHandlePickupInfo, L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupInfo", pickup_component_hooks::ServerHandlePickupInfo_Hook);

    DetourTransactionCommit();

    REGISTER_PROCESS_EVENT_DETOUR(FindObject(L"/Script/FortniteGame.FortPlayerPawn.OnCapsuleBeginOverlap") ?
        L"/Script/FortniteGame.FortPlayerPawn.OnCapsuleBeginOverlap" :
        L"/Script/FortniteGame.FortPlayerPawnAthena.OnCapsuleBeginOverlap", pickup_component_hooks::OnCapsuleBeginOverlap_Hook);
}

AFortPickup* pickup_component::spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count,
                                            int loaded_ammo, bool bToss, UObject* pickup_class, std::vector<float> GenericAttributeValues)
{
    if (item_definition == nullptr)
    {
        return nullptr;
    }
    
    static UObject* fort_pickup_class = FindObject(L"/Script/FortniteGame.FortPickupAthena");
    
    if(pickup_class == nullptr)
    {
        pickup_class = fort_pickup_class;
    }

    // Spawn pickup in world
    AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(SpawnActor(*GWorld, pickup_class, &pickup_location, nullptr, FActorSpawnParameters {}));

    if (fort_pickup == nullptr)
    {
        return nullptr;
    }

    // Finish basic initialization
    initialize_pickup(fort_pickup, item_definition, count);

    // Update item entry for the pickup
    fort_pickup->PrimaryPickupItemEntry().ItemDefinition() = item_definition;
    fort_pickup->PrimaryPickupItemEntry().Count() = count;
    fort_pickup->PrimaryPickupItemEntry().LoadedAmmo() = loaded_ammo;

    // copy list
    for (float Value : GenericAttributeValues)
    {
        if (FindPropertyByName(FFortItemEntry::GetClass(), L"GenericAttributeValues"))
        {
            fort_pickup->PrimaryPickupItemEntry().GenericAttributeValues().Add(Value);
        }
    }

    if (STATIC_HASPROPERTY(fort_pickup, bServerStoppedSimulation))
    {
        if (fort_pickup->bServerStoppedSimulation())
        {
            // Enable physics on pickups which is required on some builds
            fort_pickup->bServerStoppedSimulation() = false;
        }
    }

    if (STATIC_HASPROPERTY(fort_pickup, bRandomRotation))
    {
        fort_pickup->bRandomRotation() = true;
    }

    if (bToss == true)
    {
        // Toss pickup, prevents pickups from all spawning in the same location
        fort_pickup->TossPickup(pickup_location, nullptr, 0, true);

        handle_pickup_combination(fort_pickup);
    }
    
    return fort_pickup;
}

void pickup_component::toss_pickup(AFortPickup* fort_pickup, ABuildingActor* container, FRotator toss_direction, float toss_angle, float toss_speed)
{
    if (fort_pickup == nullptr || container == nullptr)
    {
        spdlog::error(L"pickup_component::toss_pickup failed: invalid parameters.");
        
        return;
    }

    // prevent blocking the pickup
    fort_pickup->TouchCapsule()->MoveIgnoreActors().Add(container);
    
    // First of all, start replicating movement
    UKismetSystemLibrary::SetBoolPropertyByName(fort_pickup, L"bReplicateMovement", true);

    // Convert the toss direction to a vector
    FRotator TossDirection_Rotator(container->K2_GetActorRotation().Pitch + toss_direction.Pitch,
        container->K2_GetActorRotation().Yaw + toss_direction.Yaw,
        container->K2_GetActorRotation().Roll + toss_direction.Roll);

    FVector TossDirection = UKismetMathLibrary::Conv_RotatorToVector(TossDirection_Rotator);

    // Now get a random unit vector
    FVector RandomUnitVector = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(TossDirection, toss_angle);

    // Setup the movement component for tossing
    fort_pickup->MovementComponent()->Velocity() = FVector(RandomUnitVector.X * toss_speed,
        RandomUnitVector.Y * toss_speed,
        RandomUnitVector.Z * toss_speed);

    fort_pickup->MovementComponent()->MaxSpeed() = sqrtf(fort_pickup->MovementComponent()->Velocity().X * fort_pickup->MovementComponent()->Velocity().X +
        fort_pickup->MovementComponent()->Velocity().Y * fort_pickup->MovementComponent()->Velocity().Y +
        fort_pickup->MovementComponent()->Velocity().Z * fort_pickup->MovementComponent()->Velocity().Z);

    // Now setup the pickup
    fort_pickup->bTossedFromContainer() = true;

    if (STATIC_HASPROPERTY(fort_pickup, bCombinePickupsWhenTossCompletes))
    {
        fort_pickup->bCombinePickupsWhenTossCompletes() = false;
    }
                            
    // Toss
    if (Toss != nullptr)
    {
        Toss(fort_pickup);
    }
}

void pickup_component::handle_pickup(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn)
{
    if (Pickup == nullptr)
    {
        spdlog::error(L"pickup_component::handle_pickup called with invalid Pickup");
        
        return;
    }

    // Prevent duplication
    if (Pickup->bPickedUp())
    {
        return;
    }

    // Pick it up
    InternalHandlePickup(Pickup, PlayerPawn, Pickup->PickupLocationData().FlyTime() / CreateDefaultObject<AFortPlayerPawnAthena>()->PickupSpeedMultiplier(), FVector(1,1,1));

    // Net update
    Pickup->ForceNetUpdate();
    
    // Prevent additional pickups
    Pickup->bPickedUp() = true;
}

void pickup_component::initialize_pickup(AFortPickup* pickup, UFortItemDefinition* item_definition, int count)
{
    if (InitializePickup)
    {
        __int64 OutParm[2];
        OutParm[0] = 0;
        OutParm[1] = 0;

        if (UFortWorldItem* TemporaryWorldItem = static_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(count, 1)))
        {
            FFortItemEntry* TemporaryItemEntry = &TemporaryWorldItem->ItemEntry();

            // Finish initialization
            // This function takes care of setting up, among other things, SpecialActorID and properly adds support for Special Actors
            // Special Actors are required for some items and LTMs to work, such as Infinity Gauntlet
            InitializePickup(pickup, TemporaryItemEntry, OutParm, false);
        }
    }
}

void pickup_component::handle_pickup_combination(AFortPickup* Pickup)
{
    // Data
    UFortItemDefinition* PickupItemDefinition = Pickup->PrimaryPickupItemEntry().ItemDefinition();
    
    // Construct parameters for SphereOverlapActors
    TArray<AActor*> ActorsToIgnore{};
    TArray<uint8_t> ObjectTypes;
    UEnum* EObjectTypeQuery = FindObject<UEnum>(L"/Script/Engine.EObjectTypeQuery");
    for (uint8_t i = 0; i < EObjectTypeQuery->GetEnumIndex(L"ObjectTypeQuery_MAX"); i++)
    {
        ObjectTypes.Add(i);
    }
    TArray<AActor*> Result;

    // Check for Sphere overlap
    UKismetSystemLibrary::SphereOverlapActors(*GWorld, Pickup->K2_GetActorLocation(), 128.f, ObjectTypes, AFortPickupAthena::StaticClass(), ActorsToIgnore, &Result);

    if (Result.Num() > 0)
    {
        AFortPickup* CombineTarget = nullptr;

        for (AActor* ResultInstance : Result)
        {
            AFortPickup* CurrentPickup = static_cast<AFortPickup*>(ResultInstance);
            UFortItemDefinition* CurrentPickupItemDefinition = CurrentPickup->PrimaryPickupItemEntry().ItemDefinition();
            
            if (CurrentPickup == Pickup)
            {
                continue;
            }

            if (CurrentPickup->bPickedUp())
            {
                continue;
            }

            if (CurrentPickupItemDefinition == PickupItemDefinition)
            {
                if (CurrentPickup->PrimaryPickupItemEntry().Count() < CurrentPickupItemDefinition->GetMaxStackSize())
                {
                    AFortPickup* OverrideCombineTarget = nullptr;
                    for (AFortPickup* CurrentPickupCombineTarget = CurrentPickup->PickupLocationData().CombineTarget(); CurrentPickupCombineTarget; CurrentPickupCombineTarget = CurrentPickupCombineTarget->PickupLocationData().CombineTarget())
                    {
                        OverrideCombineTarget = CurrentPickupCombineTarget;
                    }

                    if (OverrideCombineTarget)
                    {
                        if (OverrideCombineTarget->PrimaryPickupItemEntry().ItemDefinition() == PickupItemDefinition &&
                            OverrideCombineTarget->PrimaryPickupItemEntry().Count() < PickupItemDefinition->GetMaxStackSize() &&
                            OverrideCombineTarget->PickupLocationData().CombineTarget() == nullptr &&
                            OverrideCombineTarget->bPickedUp() == false)
                        {
                            CombineTarget = OverrideCombineTarget;
                            break;
                        }
                    }
                        
                    if (CombineTarget == nullptr)
                    {
                        if (CurrentPickup->PickupLocationData().CombineTarget() == nullptr)
                        {
                            CombineTarget = CurrentPickup;
                            continue;
                        }
                    }

                    if (CombineTarget)
                    {
                        float CombineTargetDistance = CombineTarget->GetDistanceTo(Pickup);
                        float PotentialCombineTargetDistance = CurrentPickup->GetDistanceTo(Pickup);

                        if (PotentialCombineTargetDistance < CombineTargetDistance)
                        {
                            if (CurrentPickup->PickupLocationData().CombineTarget() == nullptr)
                            {
                                CombineTarget = CurrentPickup;
                            }
                        }
                    }
                }
            }
        }

        if (CombineTarget)
        {
            if (STATIC_HASPROPERTY(Pickup, bCombinePickupsWhenTossCompletes))
            {
                Pickup->bCombinePickupsWhenTossCompletes() = true;
            }

            Pickup->PickupLocationData().CombineTarget() = CombineTarget;

            if (InternalCombinePickup)
            {
                Pickup->PrimaryPickupItemEntry().PreviousCount() = Pickup->PrimaryPickupItemEntry().Count();
                CombineTarget->PrimaryPickupItemEntry().PreviousCount() = CombineTarget->PrimaryPickupItemEntry().Count();

                CombineTarget->PrimaryPickupItemEntry().Count() = Pickup->PrimaryPickupItemEntry().Count() + CombineTarget->PrimaryPickupItemEntry().Count() <= PickupItemDefinition->GetMaxStackSize() ? Pickup->PrimaryPickupItemEntry().Count() + CombineTarget->PrimaryPickupItemEntry().Count() : PickupItemDefinition->GetMaxStackSize();

                if (CombineTarget->PrimaryPickupItemEntry().Count() == PickupItemDefinition->GetMaxStackSize())
                {
                    spawn_pickup(PickupItemDefinition, Pickup->K2_GetActorLocation(), Pickup->PrimaryPickupItemEntry().PreviousCount() + CombineTarget->PrimaryPickupItemEntry().PreviousCount() - PickupItemDefinition->GetMaxStackSize());
                }
                
                // Update fly time
                Pickup->PickupLocationData().FlyTime() = Pickup->PickupLocationData().FlyTime() / CreateDefaultObject<AFortPlayerPawnAthena>()->PickupSpeedMultiplier();
                
                InternalCombinePickup(Pickup);
            }
        }
    }
}

bool pickup_component_hooks::OnAboutToEnterBackpack_Hook(AFortPickup* Pickup)
{
    if (Pickup == nullptr)
    {
        return false;
    }
    
    if (AFortPawn* PickupTarget = Pickup->PickupLocationData().PickupTarget())
    {
        AFortPlayerController* PlayerController = PickupTarget->Controller();

        if (!PlayerController)
        {
            spdlog::error(L"Player attempted to pickup actor with invalid PlayerController.");

            return false;
        }
        
        if (!PlayerController->WorldInventory())
        {
            spdlog::error(L"Player attempted to pickup actor with invalid WorldInventory.");
            
            return false;
        }

        // For some reason this code breaks on 3.5, so do not run it
        if (GetCL() != 4008490)
        {
            // Check if the current pickup is found in the incoming pickups stream
            if (!PickupTarget->IncomingPickups().Find(Pickup))
            {
                return false; 
            }
        
            // If it is found, remove from the stream
            for (int i = 0; i < PickupTarget->IncomingPickups().Num(); i++)
            {
                if (PickupTarget->IncomingPickups()[i] == Pickup)
                {
                    TArrayManager<AFortPickup*>(&PickupTarget->IncomingPickups(), 8).Remove(i, FindPropertyByName(PickupTarget->Class, TEXT("IncomingPickups")));
                    break;
                }
            }
        }

        std::vector<float> GenericAttributeValues;
        if (FindPropertyByName(FFortItemEntry::GetClass(), L"GenericAttributeValues"))
        {
            for (float Value : Pickup->PrimaryPickupItemEntry().GenericAttributeValues())
            {
                GenericAttributeValues.push_back(Value);
            }
        }
        
        UFortWorldItem* spawned_item = inventory_component::add_item(PlayerController, Pickup->PrimaryPickupItemEntry().ItemDefinition(), Pickup->PrimaryPickupItemEntry().Count(), Pickup->PrimaryPickupItemEntry().LoadedAmmo(), GenericAttributeValues);

        if (inventory_component::is_inventory_full(PlayerController))
        {
            UFortItemDefinition* SwapItemDefinition = PickupTarget->CurrentWeapon()->WeaponData();
            FGuid SwapItemGuid = PickupTarget->CurrentWeapon()->ItemEntryGuid();
            bool bShouldFocusOnSwappedItem = true;
            
            if (static_cast<UFortWorldItemDefinition*>(SwapItemDefinition)->bCanBeDropped() &&
                inventory_component::has_item_type(PlayerController, SwapItemDefinition))
            {
                // Swap item
                inventory_component::remove_item(PlayerController, SwapItemGuid, -1, true, true);

                if (spawned_item)
                {
                    if (TryAddToQuickBar)
                    {
                        TryAddToQuickBar(PlayerController, spawned_item);
                    }

                    // Equip item on client
                    if (bShouldFocusOnSwappedItem)
                    {
                        PlayerController->ClientEquipItem(spawned_item->ItemEntry().ItemGuid(), true);
                    }
                }
            }
            else
            {
                if (spawned_item)
                {
                    // Unable to swap
                    inventory_component::remove_item(PlayerController, spawned_item->ItemEntry().ItemGuid(), -1);   
                }
            }
        }

        // on 3.5, run the old code
        if (GetCL() == 4008490)
        {
            Pickup->PickupLocationData().PickupTarget() = nullptr;
        }
    }
    
    return OnAboutToEnterBackpack(Pickup);
}

void pickup_component_hooks::ServerHandlePickup_Hook(AFortPlayerPawn* PlayerPawn, FFrame& StackFrame)
{
    GET_PARAMETER(AFortPickup*, Pickup);

    pickup_component::handle_pickup(Pickup, PlayerPawn);

    return ServerHandlePickup(PlayerPawn, StackFrame);
}

void pickup_component_hooks::ServerHandlePickupInfo_Hook(AFortPlayerPawn* PlayerPawn, FFrame& StackFrame)
{
    GET_PARAMETER(AFortPickup*, Pickup);

    pickup_component::handle_pickup(Pickup, PlayerPawn);

    return ServerHandlePickupInfo(PlayerPawn, StackFrame);
}

void pickup_component_hooks::OnCapsuleBeginOverlap_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AActor*, OtherActor);

    // Cast to pawn
    AFortPlayerPawn* PlayerPawn = reinterpret_cast<AFortPlayerPawn*>(Object);
    
    // Filter overlap capsule
    static UObject* fort_pickup_class = FindObject<UObject>(L"/Script/FortniteGame.FortPickupAthena");

    if (OtherActor->IsA(fort_pickup_class))
    {
        // Cast overlap capsule to FortPickup instance
        AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(OtherActor);

        // Filter pickup classes
        static UFortItemDefinition* ammo_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortAmmoItemDefinition");
        static UFortItemDefinition* resource_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortResourceItemDefinition");
        static UFortItemDefinition* trap_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortTrapItemDefinition");
        
        if (fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->IsA(ammo_definition) ||
            fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->IsA(resource_definition) ||
            fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->IsA(trap_definition))
        {
            if (fort_pickup->PickupLocationData().TossState() == 2) // AtRest
            {
                if (fort_pickup->PickupLocationData().CombineTarget() == nullptr)
                {
                    // Pickup item automatically if it is not currently being tossed
                    pickup_component::handle_pickup(fort_pickup, PlayerPawn);
                }
            }
        }
    }
}

AFortPickup* pickup_component_hooks::K2_SpawnPickupInWorld_Hook(UWorld* WorldContextObject, UFortItemDefinition* ItemDefinition, int NumberToSpawn, FVector Position, FVector Direction, int OverrideMaxStackCount, bool bToss)
{
    // Spawn the pickup TODO: Look into why the first time you throw an item, it's not loaded (Occured on 14.60)
    if(ItemDefinition)
    {
        auto NewPickup = pickup_component::spawn_pickup(ItemDefinition, Position, NumberToSpawn, looting_component::get_default_loaded_ammo(ItemDefinition), bToss);

        if (NewPickup != nullptr && bToss == false)
        {
            // Toss the pickup
            NewPickup->TossPickup(Position, nullptr, 0, false);
        
            return NewPickup;
        }   
    }
    
    return nullptr;   
}

void pickup_component_hooks::CombinePickupFinished_Hook(AFortPickup* Self)
{
    // Handle the actual combination
    if (AFortPickup* CombineTarget = Self->PickupLocationData().CombineTarget())
    {
        // Update widget
        CombineTarget->ForceNetUpdate();
        
        // Destroy Self
        Self->K2_DestroyActor();
    }
}