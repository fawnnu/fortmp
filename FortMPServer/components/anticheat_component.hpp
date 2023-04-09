#pragma once

class anticheat_component
{
public:
    anticheat_component();

    inline static std::map<AFortPlayerController*, int> IllegalActionsCount;
};

namespace anticheat_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerNotifyPawnHit, (AFortProjectileAthena* Self, FFrame& StackFrame))
}
