#pragma once

class vehicles_component
{
public:
    vehicles_component();

    static void equip_vehicle_weapon(AFortPlayerController* target_player_controller, UFortWeaponItemDefinition* weapon_item_definition);
    static void unequip_vehicle_weapon(AFortPlayerController* target_player_controller);
};

namespace vehicles_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, EquipAbilityWeapon, (UFortGameplayAbility* FortGameplayAbility, UFortWeaponItemDefinition* WeaponDefinition));
    DEFINE_FUNCTION_HOOK(void, ServerAttemptExitVehicle, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerOnExitVehicle, (AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerRequestSeatChange, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerFireActorInCannon, (AFortWeaponRangedForVehicle* Weapon, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, AFortAthenaVehicleSpawner_BeginPlay, (AFortAthenaVehicleSpawner* Target));

    // sync vehicles
    DEFINE_FUNCTION_HOOK(void, ServerUpdatePhysicsParams, (AActor* Vehicle, FFrame& StackFrame));
}

inline void (*SetWorldLocationAndRotation)(USceneComponent* RootComponent, FVector& NewLocation, FQuat& NewRotation, bool bSweep, void* OutSweepHitResult, int ETeleportType);