#pragma once

class looting_component
{
public:
    looting_component();

    static std::map<UFortItemDefinition*, int> spawn_loot(FName loot_category);

    static void set_vending_machine_loot(ABuildingItemCollectorActor* vending_machine);

    static FFortLootTierData* get_random_loot_tier_data(FName loot_category);
    static FFortLootPackageData* get_random_loot_package_data(FFortLootTierData* loot_tier_data);

    static UObject* get_loot_tier_data();
    static UObject* get_loot_tier_packages();
    static int get_default_loaded_ammo(UFortItemDefinition* in_item_definition);

    inline static std::vector<UFortItemDefinition*> blacklisted_item_definitions;
    inline static std::vector<UObject*> blacklisted_classes;
};

namespace looting_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, PickLootDrops, (TArray<FFortItemEntry>*, FName));
    DEFINE_FUNCTION_HOOK(void, PickLootDrops_New, (UWorld*, TArray<FFortItemEntry>*, FName));

    DEFINE_FUNCTION_HOOK(AFortPickup*, AFortAthenaSupplyDrop_SpawnPickup, (AFortAthenaSupplyDrop* FortAthenaSupplyDrop, UFortItemDefinition* ItemDefinition, int NumberToSpawn, AFortPawn* TriggeringPawn, FVector* Position));

    DEFINE_FUNCTION_HOOK(AFortGameModePickup*, AFortAthenaSupplyDrop_SpawnGameModePickup, (AFortAthenaSupplyDrop* FortAthenaSupplyDrop, UFortItemDefinition* ItemDefinition, UObject* PickupClass, int NumberToSpawn, AFortPawn* TriggeringPawn, FVector* Position, FVector* Direction));
}

inline AFortAthenaSupplyDrop* (*SpawnSupplyDrop)(AFortAthenaMapInfo* FortAthenaMapInfo, void* UnknownParam, int UnknownParam_1, int SafezoneIndex, UFortSupplyDropInfo* FortSupplyDropInfo, FRotator* InRotation);