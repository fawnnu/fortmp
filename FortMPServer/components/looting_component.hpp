#pragma once

class looting_component
{
public:
    looting_component();

    static std::map<UFortItemDefinition*, int> spawn_loot(FString loot_category);

    static UObject* get_loot_tier_data();
    static UObject* get_loot_tier_packages();

    inline static std::vector<UFortItemDefinition*> blacklisted_item_definitions;
    inline static std::vector<UObject*> blacklisted_classes;
};

namespace looting_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, OnDamageServer, (UObject* Object, UObject* Function, void* Params));
}
