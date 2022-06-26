#include "include.hpp"
#include "pickup_component.hpp"

pickup_component::pickup_component()
{
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

    // Update the loaded ammo within the pickup
    fort_pickup->PrimaryPickupItemEntry().LoadedAmmo() = loaded_ammo;

    // TODO: Add bServerStoppedSimulation, it might need a is_bad_read_ptr as the property does not exist on some builds
    fort_pickup->bRandomRotation() = true;

    // Toss pickup, prevents pickups from all spawning in the same location
    fort_pickup->TossPickup(pickup_location, nullptr, 0, true);

    return fort_pickup;
}