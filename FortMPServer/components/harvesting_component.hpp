#pragma once

class harvesting_component
{
public:
    harvesting_component();

    static void handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage);
};

namespace harvesting_component_hooks
{
    // Called when a player damages an actor
    DEFINE_FUNCTION_HOOK(void, NetMulticast_Athena_BatchedDamageCues, (UObject* Object, UObject* Function, void* Params));
}