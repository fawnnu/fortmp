#pragma once

class vehicles_component
{
public:
    vehicles_component();
};

namespace vehicles_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, EquipAbilityWeapon, (UFortGameplayAbility* FortGameplayAbility, UFortWeaponItemDefinition* WeaponDefinition));
    DEFINE_FUNCTION_HOOK(void, ServerAttemptExitVehicle, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // sync vehicles
    DEFINE_FUNCTION_HOOK(void, ServerUpdatePhysicsParams, (AActor* Vehicle, FFrame& StackFrame));
}

inline void (*SetWorldLocationAndRotation)(USceneComponent* RootComponent, FVector& NewLocation, FQuat& NewRotation, bool bSweep, void* OutSweepHitResult, int ETeleportType);