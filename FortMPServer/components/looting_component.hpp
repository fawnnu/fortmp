#pragma once

class looting_component
{
public:
    looting_component();

    // Picks loot for a given tier group name
    static std::map<UFortItemDefinition*, int> spawn_loot(FName loot_category);

    // Convert a tier group name to an athena one
    static FName redirect_tier_group(FName InTierGroup);

    // Picks loot for vending machines
    static void set_vending_machine_loot(ABuildingItemCollectorActor* vending_machine);

    // Get a random Loot Tier Data of a given category
    static FFortLootTierData* get_random_loot_tier_data(FName loot_category);

    // Get a random Loot package Data of a given Tier Data
    static FFortLootPackageData* get_random_loot_package_data(FFortLootTierData* loot_tier_data);
    
    // Get a random Loot package Data of a given LootPackageID
    static FFortLootPackageData* get_random_loot_package_data_from_id(UDataTable* DataTable, FName loot_package_id);

    // Get Loot Tier Data Datatable from playlist, or default
    static UObject* get_loot_tier_data();

    // Get Loot Tier Packages from playlist, or default
    static UObject* get_loot_tier_packages();

    // Get an ItemDefinition's default loaded ammo
    static int get_default_loaded_ammo(UFortItemDefinition* in_item_definition);

private:

    // blacklisted item defintions when picking loot
    inline static std::vector<UFortItemDefinition*> blacklisted_item_definitions;

    // blacklisted classes when picking loot
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