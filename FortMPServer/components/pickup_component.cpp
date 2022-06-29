#include "include.hpp"
#include "pickup_component.hpp"

#include "inventory_component.hpp"

pickup_component::pickup_component()
{
    INIT_POINTER(GrabPickup, memory_manager::find_function_by_string(L"Attempted to spawn non-world item %s!"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(pickup_component_hooks::ServerHandlePickup, L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup", pickup_component_hooks::ServerHandlePickup_Hook);

    DetourTransactionCommit();
}

AFortPickup* pickup_component::spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count,
                                            int loaded_ammo)
{
    if (item_definition == nullptr)
    {
        return nullptr;
    }
    
    static UObject* fort_pickup_class = FindObject(L"/Script/FortniteGame.FortPickup");

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

    if (fort_pickup->HasProperty("bServerStoppedSimulation"))
    {
        if (fort_pickup->bServerStoppedSimulation())
        {
            // Enable physics on pickups which is required on some builds
            fort_pickup->bServerStoppedSimulation() = false;
        }
    }
    
    // TODO: Add bServerStoppedSimulation, it might need a is_bad_read_ptr as the property does not exist on some builds
    fort_pickup->bRandomRotation() = true;

    // Toss pickup, prevents pickups from all spawning in the same location
    fort_pickup->TossPickup(pickup_location, nullptr, 0, true);

    return fort_pickup;
}

void pickup_component_hooks::ServerHandlePickup_Hook(AFortPlayerPawn* PlayerPawn, FFrame& StackFrame)
{
    GET_PARAMETER(AFortPickup*, Pickup);
    GET_PARAMETER(float, InFlyTime);
    GET_PARAMETER(FVector, InStartDirection);
    GET_PARAMETER(bool, bPlayPickupSound);

    if (Pickup == nullptr)
    {
        return;
    }

    if (Pickup->bPickedUp())
    {
        // Prevent duplication
        return;
    }

    UFortWorldItem* spawned_item = inventory_component::add_item(PlayerPawn->Controller(), Pickup->PrimaryPickupItemEntry().ItemDefinition(), Pickup->PrimaryPickupItemEntry().Count(), Pickup->PrimaryPickupItemEntry().LoadedAmmo());

    if (spawned_item == nullptr)
    {
        return;
    }

    if (inventory_component::is_inventory_full(PlayerPawn->Controller()))
    {
        static UObject* melee_item_definition_class = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");

        if (!PlayerPawn->CurrentWeapon()->WeaponData()->IsA(melee_item_definition_class))
        {
            // Swap item
            inventory_component::remove_item(PlayerPawn->Controller(), PlayerPawn->CurrentWeapon()->ItemEntryGuid(), -1);

            // Equip item on client
            PlayerPawn->Controller()->ClientEquipItem(spawned_item->ItemEntry().ItemGuid(), true);
        }
        else
        {
            // Unable to swap
            inventory_component::remove_item(PlayerPawn->Controller(), spawned_item->ItemEntry().ItemGuid(), -1);
        }
    }

    // Grab pickup
    GrabPickup(Pickup, PlayerPawn, InFlyTime, 0);

    // Prevent additional pickups
    Pickup->bPickedUp() = true;
    
    return ServerHandlePickup(PlayerPawn, StackFrame);
}
