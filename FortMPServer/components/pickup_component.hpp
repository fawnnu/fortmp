#pragma once

class pickup_component
{
public:
    pickup_component();
    
    static AFortPickup* spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count, int loaded_ammo = 0);
};

namespace pickup_component_hooks
{
    // Called when a player tries to pickup an item
    DEFINE_FUNCTION_HOOK(void, ServerHandlePickup, (AFortPlayerPawn* PlayerPawn, FFrame& StackFrame));

    // Called when a player overlaps with a capsule component
    DEFINE_FUNCTION_HOOK(void, OnCapsuleBeginOverlap, (UObject* Object, UObject* Function, void* Params));
}

// Used to play the "grab" animation for pickups
inline void (*GrabPickup)(UObject*, UObject*, float InFlyTime, __int64 a4);