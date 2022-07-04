#pragma once

class looting_component
{
public:
    looting_component();

    static std::map<UFortItemDefinition*, int> spawn_loot(FString loot_category);

    static UObject* get_loot_tier_data();
    static UObject* get_loot_tier_packages();
};

namespace looting_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, OnDamageServer, (UObject* Object, UObject* Function, void* Params));
}
