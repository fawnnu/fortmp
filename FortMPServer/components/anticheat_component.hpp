#pragma once

class anticheat_component
{
public:
    anticheat_component();

    inline static std::map<AFortPlayerController*, int> IllegalSniperActionsCount;
    inline static std::map<AFortWeapon*, int> IllegalWeaponActionsCount;
    inline static std::map<AFortWeapon*, float> LastFireTime;
};

namespace anticheat_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerNotifyPawnHit, (AFortProjectileAthena* Self, FFrame& StackFrame));
}
