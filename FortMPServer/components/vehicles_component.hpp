#pragma once

class vehicles_component
{
public:
    vehicles_component();
};

namespace vehicles_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, EquipAbilityWeapon, (UFortGameplayAbility* FortGameplayAbility, FFrame& StackFrame));
}
