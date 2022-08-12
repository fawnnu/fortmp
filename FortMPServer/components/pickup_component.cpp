#include "include.hpp"
#include "pickup_component.hpp"

#include "inventory_component.hpp"

pickup_component::pickup_component()
{
    INIT_POINTER(GrabPickup, memory_manager::find_function_by_string(L"Attempted to spawn non-world item %s!"));

    // On chapter 2, the pickup anim function has been changed. We can now access it by its VTable index
    if (uintptr_t grab_pickup_addr = memory_manager::find_pattern("FF 93 ? ? ? ? 49 63 8F ? ? ? ? 85 C9 0F 84 ? ? ? ? 49 8B 87 ? ? ? ? 48 89 AC 24 ? ? ? ? 48 89 B4 24 ? ? ? ? 48 8B F1"))
    {
        //Init the pickup anim function by accessing FortPickupAthena VTable at the obtained index
        INIT_POINTER(GrabPickup, FindObject(L"/Script/FortniteGame.Default__FortPickupAthena")->VTable[*reinterpret_cast<int*>(grab_pickup_addr + 2) / 8] );
    }
    
    // REGISTER_PROCESS_EVENT_DETOUR(L"OnCapsuleBeginOverlap", pickup_component_hooks::OnCapsuleBeginOverlap_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(pickup_component_hooks::ServerHandlePickup, L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup", pickup_component_hooks::ServerHandlePickup_Hook);
    REGISTER_UFUNCTION_DETOUR(pickup_component_hooks::ServerHandlePickupInfo, L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupInfo", pickup_component_hooks::ServerHandlePickupInfo_Hook);

    REGISTER_FUNCTION_DETOUR(pickup_component_hooks::NotifyActorDestroyed, memory_manager::find_function_by_string(L"NotifyActorDestroyed %s - StartupActor"), pickup_component_hooks::NotifyActorDestroyed_Hook);
    
    DetourTransactionCommit();
}

AFortPickup* pickup_component::spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count,
                                            int loaded_ammo)
{
    if (item_definition == nullptr)
    {
        return nullptr;
    }
    
    static UObject* fort_pickup_class = FindObject(L"/Script/FortniteGame.FortPickupAthena");

    // Spawn pickup in world
    AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(SpawnActor(*GWorld, fort_pickup_class, &pickup_location, nullptr, FActorSpawnParameters {})); // <--- crashes

    if (fort_pickup == nullptr)
    {
        return nullptr;
    }

    // Update item entry for the pickup
    fort_pickup->PrimaryPickupItemEntry().ItemDefinition() = item_definition;
    fort_pickup->PrimaryPickupItemEntry().Count() = count;
    fort_pickup->PrimaryPickupItemEntry().LoadedAmmo() = loaded_ammo;

    // make variables static for performance
    static bool b_has_bserverstoppedsimulation = fort_pickup->HasProperty("bServerStoppedSimulation");
    
    if (b_has_bserverstoppedsimulation)
    {
        if (fort_pickup->bServerStoppedSimulation())
        {
            // Enable physics on pickups which is required on some builds
            fort_pickup->bServerStoppedSimulation() = false;
        }
    }

    static bool b_has_brandomrotation = fort_pickup->HasProperty(L"bRandomRotation");
    
    if (b_has_brandomrotation)
    {
        fort_pickup->bRandomRotation() = true;
    }
    
    // Toss pickup, prevents pickups from all spawning in the same location
    fort_pickup->TossPickup(pickup_location, nullptr, 0, true);

    return fort_pickup;
}

void pickup_component::handle_pickup(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn)
{
    if (Pickup == nullptr)
    {
        return;
    }

    if (Pickup->bPickedUp())
    {
        // Prevent duplication
        return;
    }
    
    __int64 grab_pickup_parameter = _mm_unpacklo_ps({}, {}).m128_u64[0]; //taken from IDA

    static UFunction* GrabPickupCustomized = FindObject<UFunction>(L"/Script/FortniteGame.FortAIPawn.GrabPickupCustomized");
    
    //Use fortnite UFunction when possible
    if (GrabPickupCustomized)
    {
        reinterpret_cast<AFortAIPawn*>(PlayerPawn)->GrabPickupCustomized(Pickup, 0.4f);
    }
    else
    {
        GrabPickup(Pickup, PlayerPawn, 0.4f, reinterpret_cast<__int64>(&grab_pickup_parameter));
    }
    
    // Prevent additional pickups
    Pickup->bPickedUp() = true;

    // Set PickupTarget for later
    Pickup->PickupLocationData().PickupTarget() = PlayerPawn;
}

void pickup_component_hooks::NotifyActorDestroyed_Hook(UNetDriver* _this, AActor* ThisActor, bool bSeamlessTravel)
{
    static UObject* FortPickupAthena_Class = FindObject(L"/Script/FortniteGame.FortPickupAthena");

    // Pickup delay
    if (ThisActor->IsA(FortPickupAthena_Class))
    {
        AFortPickup* Pickup = static_cast<AFortPickup*>(ThisActor);
        AFortPawn* PlayerPawn = Pickup->PickupLocationData().PickupTarget();

        if (!PlayerPawn)
        {
            spdlog::error(L"Played attempted to pickup actor with invalid PlayerPawn.");
            
            return NotifyActorDestroyed(_this, ThisActor, bSeamlessTravel);
        }
        
        if (!PlayerPawn->Controller()->WorldInventory())
        {
            spdlog::error(L"Played attempted to pickup actor with invalid WorldInventory.");
            
            return NotifyActorDestroyed(_this, ThisActor, bSeamlessTravel);
        }
        
        UFortWorldItem* spawned_item = inventory_component::add_item(PlayerPawn->Controller(), Pickup->PrimaryPickupItemEntry().ItemDefinition(), Pickup->PrimaryPickupItemEntry().Count(), Pickup->PrimaryPickupItemEntry().LoadedAmmo());

        if (inventory_component::is_inventory_full(PlayerPawn->Controller()))
        {
            static UObject* melee_item_definition_class = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");

            if (!PlayerPawn->CurrentWeapon()->WeaponData()->IsA(melee_item_definition_class))
            {
                // Swap item
                inventory_component::remove_item(PlayerPawn->Controller(), PlayerPawn->CurrentWeapon()->ItemEntryGuid(), -1, true, true);

                static bool b_legacy_quickbars = PlayerPawn->Controller()->HasProperty(L"QuickBars");
    
                if (b_legacy_quickbars)
                {
                    PlayerPawn->Controller()->QuickBars()->ServerAddItemInternal(spawned_item->ItemEntry().ItemDefinition(), spawned_item->ItemEntry().ItemGuid());
                }
            
                // Equip item on client
                if (spawned_item != nullptr)
                {
                    PlayerPawn->Controller()->ClientEquipItem(spawned_item->ItemEntry().ItemGuid(), true);
                }
            }
            else
            {
                if (spawned_item != nullptr)
                {
                    // Unable to swap
                    inventory_component::remove_item(PlayerPawn->Controller(), spawned_item->ItemEntry().ItemGuid(), -1);   
                }
            }
        }

        Pickup->PickupLocationData().PickupTarget() = nullptr;
    }

    NotifyActorDestroyed(_this, ThisActor, bSeamlessTravel);
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
        static AFortPickup* fort_pickup = reinterpret_cast<AFortPickup*>(OtherActor);

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
                PlayerPawn->ServerHandlePickup(fort_pickup, 0.f, {}, false);
            }
        }
    }
}
