#include "include.hpp"
#include "pickup_component.hpp"

#include "inventory_component.hpp"
#include "looting_component.hpp"

pickup_component::pickup_component()
{
    INIT_POINTER(Toss, memory_manager::get_function_start(memory_manager::find_pattern("E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B CB 48 8B 5C 24 ? 48 83 C4 20 5F")));

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
                                            int loaded_ammo, bool bToss, UObject* pickup_class)
{
    if (item_definition == nullptr)
    {
        return nullptr;
    }
    
    if(pickup_class == nullptr)
    {
        pickup_class = FindObject(L"/Script/FortniteGame.FortPickupAthena");
    }

    // Spawn pickup in world
    AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(SpawnActor(*GWorld, pickup_class, &pickup_location, nullptr, FActorSpawnParameters {}));

    if (fort_pickup == nullptr)
    {
        return nullptr;
    }

    // Update item entry for the pickup
    fort_pickup->PrimaryPickupItemEntry().ItemDefinition() = item_definition;
    fort_pickup->PrimaryPickupItemEntry().Count() = count;
    fort_pickup->PrimaryPickupItemEntry().LoadedAmmo() = loaded_ammo;

    if (static bool HasProperty = fort_pickup->HasProperty("bServerStoppedSimulation"))
    {
        if (fort_pickup->bServerStoppedSimulation())
        {
            // Enable physics on pickups which is required on some builds
            fort_pickup->bServerStoppedSimulation() = false;
        }
    }

    if (static bool HasProperty = fort_pickup->HasProperty(L"bRandomRotation"))
    {
        fort_pickup->bRandomRotation() = true;
    }

    if (bToss == true)
    {
        // Toss pickup, prevents pickups from all spawning in the same location
        fort_pickup->TossPickup(pickup_location, nullptr, 0, true);
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

    if (fort_pickup->HasProperty("bCombinePickupsWhenTossCompletes"))
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
        spdlog::error(L"Attempting to pick up {}, but bPickedUp is true", Pickup->GetName().c_str());
        
        return;
    }
    
    // Set the pickup target and fly time
    Pickup->PickupLocationData().PickupTarget() = PlayerPawn;
    Pickup->PickupLocationData().FlyTime() /= static_cast<AFortPlayerPawnAthena*>(PlayerPawn)->PickupSpeedMultiplier();
    Pickup->PickupLocationData().ItemOwner() = PlayerPawn;
    
    // Net update
    Pickup->ForceNetUpdate();
    
    // Trigger the pickup anim
    Pickup->OnRep_PickupLocationData();
    
    // Prevent additional pickups
    Pickup->bPickedUp() = true;
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
        
        UFortWorldItem* spawned_item = inventory_component::add_item(PlayerController, Pickup->PrimaryPickupItemEntry().ItemDefinition(), Pickup->PrimaryPickupItemEntry().Count(), Pickup->PrimaryPickupItemEntry().LoadedAmmo());

        if (inventory_component::is_inventory_full(PlayerController))
        {
            static UObject* melee_item_definition = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
            static UObject* building_item_definition = FindObject(L"/Script/FortniteGame.FortBuildingItemDefinition");
            static UObject* edit_tool_definition = FindObject(L"/Script/FortniteGame.FortEditToolItemDefinition");

            if (!PickupTarget->CurrentWeapon()->WeaponData()->IsA(melee_item_definition) &&
                !PickupTarget->CurrentWeapon()->WeaponData()->IsA(building_item_definition) &&
                !PickupTarget->CurrentWeapon()->WeaponData()->IsA(edit_tool_definition))
            {
                // Swap item
                inventory_component::remove_item(PlayerController, PickupTarget->CurrentWeapon()->ItemEntryGuid(), -1, true, true);

                if (spawned_item != nullptr)
                {
                    if (TryAddToQuickBar != nullptr)
                    {
                        TryAddToQuickBar(PlayerController, spawned_item);
                    }
            
                    // Equip item on client
                    PlayerController->ClientEquipItem(spawned_item->ItemEntry().ItemGuid(), true);
                }
            }
            else
            {
                if (spawned_item != nullptr)
                {
                    // Unable to swap
                    inventory_component::remove_item(PlayerController, spawned_item->ItemEntry().ItemGuid(), -1);   
                }
            }
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

    if (UKismetMathLibrary::ClassIsChildOf(OtherActor->Class, fort_pickup_class))
    {
        // Cast overlap capsule to FortPickup instance
        AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(OtherActor);

        // Filter pickup classes
        static UFortItemDefinition* ammo_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortAmmoItemDefinition");
        static UFortItemDefinition* resource_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortResourceItemDefinition");
        static UFortItemDefinition* trap_definition = FindObject<UFortItemDefinition>(L"/Script/FortniteGame.FortTrapItemDefinition");
        
        if (UKismetMathLibrary::ClassIsChildOf(fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->Class, ammo_definition) ||
            UKismetMathLibrary::ClassIsChildOf(fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->Class, resource_definition) ||
            UKismetMathLibrary::ClassIsChildOf(fort_pickup->PrimaryPickupItemEntry().ItemDefinition()->Class, trap_definition))
        {
            if (fort_pickup->PickupLocationData().TossState() == 2) // AtRest
            {
                // Pickup item automatically if it is not currently being tossed
                pickup_component::handle_pickup(fort_pickup, PlayerPawn);
            }
        }
    }
}

AFortPickup* pickup_component_hooks::K2_SpawnPickupInWorld_Hook(UWorld* WorldContextObject, UFortItemDefinition* ItemDefinition, int NumberToSpawn, FVector Position, FVector Direction, int OverrideMaxStackCount, bool bToss)
{
    // Spawn the pickup
    auto NewPickup = pickup_component::spawn_pickup(ItemDefinition, Position, NumberToSpawn, looting_component::get_default_loaded_ammo(ItemDefinition), bToss);

    if (NewPickup != nullptr && bToss == false)
    {
            // Toss the pickup
        NewPickup->TossPickup(Position, nullptr, 0, false);
        
        return NewPickup;
    }
    
    return nullptr;   
}