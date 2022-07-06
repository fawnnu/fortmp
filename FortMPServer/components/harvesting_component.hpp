#pragma once

class harvesting_component
{
public:
    harvesting_component();

    static void handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage);
};

namespace harvesting_component_hooks
{
    // Called when a player damages any building actor
    DEFINE_FUNCTION_HOOK(void, OnDamageServer, (UObject* Object, UObject* Function, void* Params));

    // Called for blueprints to modify incoming damage on a building prop
    DEFINE_FUNCTION_HOOK(void, BlueprintModifyIncomingDamage, (UObject* Object, UObject* Function, void* Params));
}