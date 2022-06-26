#pragma once

class looting_component
{
public:
    looting_component();

    static std::map<UFortItemDefinition*, int> spawn_loot(FString loot_category);

    static UObject* get_loot_tier_data();
    static UObject* get_loot_tier_packages();
};
