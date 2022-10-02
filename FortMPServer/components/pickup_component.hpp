#pragma once

class pickup_component
{
public:
    pickup_component();
    
    static AFortPickup* spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count, int loaded_ammo = 0, bool bToss = true);

    static void toss_pickup(AFortPickup* fort_pickup, ABuildingActor* from_container, FRotator toss_direction, float toss_angle, float toss_speed);

    static void handle_pickup(AFortPickup* pickup, AFortPlayerPawn* PlayerPawn);
};

namespace pickup_component_hooks
{
    // Called when a player tries to pickup an item
    DEFINE_FUNCTION_HOOK(void, ServerHandlePickup, (AFortPlayerPawn* PlayerPawn, FFrame& StackFrame));

    // Called when a player tries to pickup an item on season 13+
    DEFINE_FUNCTION_HOOK(void, ServerHandlePickupInfo, (AFortPlayerPawn* PlayerPawn, FFrame& StackFrame));

    // Called when a player overlaps with a capsule component
    DEFINE_FUNCTION_HOOK(void, OnCapsuleBeginOverlap, (UObject* Object, UObject* Function, void* Params));
    
    // Called when a pickup should be added to a player's inventory
    DEFINE_FUNCTION_HOOK(bool, OnAboutToEnterBackpack, (AFortPickup* Pickup));

    // Called when a pickup should be spawned
    DEFINE_FUNCTION_HOOK(AFortPickup*, K2_SpawnPickupInWorld, (UWorld* WorldContextObject, UFortItemDefinition* ItemDefinition, int NumberToSpawn, FVector Position));
}

inline void (*Toss)(AFortPickup*);