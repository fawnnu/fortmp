#pragma once

class harvesting_component
{
public:
    harvesting_component();

    static void handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage);
};

namespace harvesting_component_hooks
{
    // Called when a player damages a building actor
    DEFINE_FUNCTION_HOOK(void, OnBuildingDamaged, (ABuildingActor* BuildingActor, float Damage, __int64 a3, __int64 a4, UObject* Instigatedby, AActor* DamagedBy));
}