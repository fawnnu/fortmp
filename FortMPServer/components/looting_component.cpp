#include "include.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "inventory_component.hpp"

#include <random>

#include "reviving_component.hpp"

looting_component::looting_component()
{
    INIT_POINTER(SpawnSupplyDrop, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 60 49 63 F1 48 8B E9 4D 63 F0 45 85 C9 78 46 3B B1 ? ? ? ?"));
    
    if (UFunction* target_function = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.PickLootDrops"))
    {
        auto stripped_calls = memory_manager::get_all_stripped_calls_in_function(reinterpret_cast<uintptr_t>(target_function->get_native_function()));

        REGISTER_CALL_DETOUR_WITH_ADDRESS(stripped_calls[stripped_calls.size() - 1], FindPropertyByName(target_function, L"WorldContextObject") ? reinterpret_cast<void*>(looting_component_hooks::PickLootDrops_New_Hook) : reinterpret_cast<void*>(looting_component_hooks::PickLootDrops_Hook));
    }
    
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaSupplyDrop.SpawnPickup", looting_component_hooks::AFortAthenaSupplyDrop_SpawnPickup_Hook);
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaSupplyDrop.SpawnGameModePickup", looting_component_hooks::AFortAthenaSupplyDrop_SpawnGameModePickup_Hook);
    
    blacklisted_item_definitions = {};
    blacklisted_classes = {FindObject(L"/Script/FortniteGame.FortGadgetItemDefinition", true)};
}

std::map<UFortItemDefinition*, int> looting_component::spawn_loot(FName loot_category)
{
    UObject* loot_tier_data = get_loot_tier_data();
    UObject* loot_tier_packages = get_loot_tier_packages();
    
    // A map of all loot rolled to spawn in the world
    std::map<UFortItemDefinition*, int> loot_to_spawn;
    
    if (loot_tier_data == nullptr || loot_tier_packages == nullptr)
    {
        return loot_to_spawn;
    }

    TMap<FName, FFortLootTierData*> loot_tier_data_map = *reinterpret_cast<TMap<FName, FFortLootTierData*>*>(__int64(loot_tier_data) + 0x30);
    TMap<FName, FFortLootPackageData*> loot_tier_packages_map = *reinterpret_cast<TMap<FName, FFortLootPackageData*>*>(__int64(loot_tier_packages) + 0x30);

    // Row name and weight
    std::vector<FName> category_row_names;
    std::vector<FName> loot_package_calls;

    float max_weight = 0.0f;

    for (auto element : loot_tier_data_map)
    {
        // Category name
        FName category_row_name = element.Element.Key;

        // Loot tier data for the category
        FFortLootTierData* category_loot_tier_data = element.Element.Value;
        
        if (category_loot_tier_data->TierGroup() == loot_category)
        {
            if (category_loot_tier_data->Weight() > 0.0f)
            {
                category_row_names.push_back(category_row_name);
            }
        }
    }

    for (FName category_row_name : category_row_names)
    {
        // Increase max weight based on each individual category weight
        max_weight += loot_tier_data_map[category_row_name].Value->Weight();
    }

    FFortLootTierData* final_loot_tier_data = nullptr;

    // Generate random float between zero and the weight calculated
    float random_weight = UKismetMathLibrary::RandomFloatInRange(0.0f, max_weight);
    
    for (FName category_row_name : category_row_names)
    {
        if (random_weight < loot_tier_data_map[category_row_name].Value->Weight())
        {
            // Choose this row
            final_loot_tier_data = loot_tier_data_map[category_row_name].Value;

            break;
        }

        random_weight -= loot_tier_data_map[category_row_name].Value->Weight();
    }

    if (final_loot_tier_data == nullptr)
    {
        return loot_to_spawn;
    }

    category_row_names.clear();
    
    for (auto element : loot_tier_packages_map)
    {
        // Current package name
        FName loot_package_name = element.Element.Key;
        
        // Current loot package data
        FFortLootPackageData* loot_package_data = element.Element.Value;

        if (loot_package_data->LootPackageID().ToString() == final_loot_tier_data->LootPackage().ToString())
        {
            // Add current loot package data to list of names
            category_row_names.push_back(loot_package_name);
        }
    }

    // Only call ToString once for performance
    std::wstring final_loot_package = final_loot_tier_data->LootPackage().ToString();

    //fixes an issue with datatables' NumLootPackageDrops on chapter 2
    static bool b_is_chapter_two = GetEngineVersionEasy() > 423;

    float NumLootPackageDrops = 0;

    if (b_is_chapter_two)
    {
        NumLootPackageDrops = final_loot_tier_data->NumLootPackageDrops() + 1;
    }
    else
    {
        NumLootPackageDrops = final_loot_tier_data->NumLootPackageDrops();
    }

    for (float i = 0; i < NumLootPackageDrops; i++)
    {
        auto name_iterator = category_row_names.begin();

        // Advance iterator by current number of loot package drops
        // Why not just use category_row_names[i] or am I missing something?
        std::advance(name_iterator, i);

        if (name_iterator == category_row_names.end())
        {
            break;
        }

        std::vector<FName> temp_loot_package_calls;

        if (!wcsstr(final_loot_package.c_str(), L"WorldList"))
        {
            for (auto element : loot_tier_packages_map)
            {
                // Current loot package name
                FName loot_package_name = element.Element.Key;
                
                // Current loot package data
                FFortLootPackageData* loot_package_data = element.Element.Value;

                if (loot_package_data->LootPackageID().ToString() == loot_tier_packages_map[*name_iterator].Value->LootPackageCall().c_str())
                {
                    // Add to temp loot packages
                    temp_loot_package_calls.push_back(loot_package_name);
                }
            }
        }
        else if (wcsstr(final_loot_package.c_str(), L"Empty"))
        {
            // Empty loot package name, do nothing
            continue;
        }
        else
        {
            for (FName category_row_name : category_row_names)
            {
                // Add to temp loot packages
                temp_loot_package_calls.push_back(category_row_name);
            }
        }

        float temp_max_weight = 0.0f;
        
        for (FName loot_package_name : temp_loot_package_calls)
        {
            // Add current loot package to temp weight
            temp_max_weight += loot_tier_packages_map[loot_package_name].Value->Weight();
        }
        
        // Generate random float between zero and the weight calculated
        float temp_random_weight = UKismetMathLibrary::RandomFloatInRange(0.0f, temp_max_weight);

        for (FName loot_package_name : temp_loot_package_calls)
        {
            if (temp_random_weight < loot_tier_packages_map[loot_package_name].Value->Weight())
            {
                // Add to loot packages
                loot_package_calls.push_back(loot_package_name);

                break;
            }

            temp_random_weight -= loot_tier_packages_map[loot_package_name].Value->Weight();
        }
    }
    
    for (FName loot_package_name : loot_package_calls)
    {
        // Get the item definition to spawn for loot
        UFortItemDefinition* item_definition = loot_tier_packages_map[loot_package_name].Value->ItemDefinition().Get();

        if (item_definition == nullptr)
        {
            continue;
        }

        bool b_found_blacklisted_item = false;

        for (UFortItemDefinition* blacklisted_item : blacklisted_item_definitions)
        {
            if (blacklisted_item == item_definition)
            {
                b_found_blacklisted_item = true;
                break;
            }
        }

        for (UObject* blacklisted_class : blacklisted_classes)
        {
            if (item_definition->IsA(blacklisted_class))
            {
                b_found_blacklisted_item = true;
                break;
            }
        }

        if (b_found_blacklisted_item)
        {
            spdlog::info(L"Found blacklisted item '{}', not spawning.", item_definition->GetName().c_str());
            
            continue;
        }

        // Add chosen item to rolled loot
        loot_to_spawn[item_definition] = loot_tier_packages_map[loot_package_name].Value->Count();
    }
    
    return loot_to_spawn;
}

FName looting_component::redirect_tier_group(FName InTierGroup)
{
    // Convert InTierGroup if necessary
    if (InTierGroup == L"Loot_Treasure")
    {
        return L"Loot_AthenaTreasure";
    }
    if (InTierGroup == L"Loot_Ammo")
    {
        return L"Loot_AthenaAmmoLarge";
    }

    // Return anything that doesn't have to be converted
    return InTierGroup;
}

void looting_component::set_vending_machine_loot(ABuildingItemCollectorActor* vending_machine)
{
    // get our datatables at first
    UObject* loot_tier_data = get_loot_tier_data();
    UObject* loot_tier_packages = get_loot_tier_packages();
    
    if (loot_tier_data == nullptr || loot_tier_packages == nullptr)
    {
        return;
    }

    UE::TMap<FName, FFortLootTierData*> loot_tier_data_map = *reinterpret_cast<UE::TMap<FName, FFortLootTierData*>*>(__int64(loot_tier_data) + 0x30);
    UE::TMap<FName, FFortLootPackageData*> loot_tier_packages_map = *reinterpret_cast<UE::TMap<FName, FFortLootPackageData*>*>(__int64(loot_tier_packages) + 0x30);

    // Save all the Loot_AthenaVending instances
    std::vector<FName> Loot_AthenaVendings;
    std::vector<FFortLootTierData*> Loot_AthenaVendings_WithTier;
    
    // filter the loot_tier_data_map with Loot_AthenaVending with a valid weight
    for (auto element : loot_tier_data_map)
    {
        FFortLootTierData* category_loot_tier_data = element.Value();
        
        if (category_loot_tier_data->TierGroup() == L"Loot_AthenaVending")
        {
            if (category_loot_tier_data->Weight() > 0.f)
            {
                Loot_AthenaVendings.push_back(element.Key());
            }
        }
    }

    // Now pick a random row to get a random Tier
    int picked_tier = 0;

    {
        float max_weight = 0.f;

        for (FName row : Loot_AthenaVendings)
        {
            max_weight += loot_tier_data_map[row]->Weight();
        }

        // Random weight
        float random_weight = UKismetMathLibrary::RandomFloatInRange(0.f, max_weight);

        for (FName row : Loot_AthenaVendings)
        {
            if (random_weight < loot_tier_data_map[row]->Weight())
            {
                picked_tier = loot_tier_data_map[row]->LootTier();
            
                break;
            }

            random_weight -= loot_tier_data_map[row]->Weight();
        }
    }

    // Now filter the Loot_AthenaVendings to get only one different tier
    for (FName row : Loot_AthenaVendings)
    {
        if (loot_tier_data_map[row]->LootTier() == picked_tier)
        {
            Loot_AthenaVendings_WithTier.push_back(loot_tier_data_map[row]);
        }
    }

    // now pick three loot tier datas from Loot_AthenaVendings_WithTier
    std::vector<FFortLootTierData*> PickedLootTierData;

    auto PickRandomLootTierData = [&]
    {
        float max_weight = 0.f;

        for (FFortLootTierData* tier_data : Loot_AthenaVendings_WithTier)
        {
            max_weight += tier_data->Weight();
        }

        // Random weight
        float random_weight = UKismetMathLibrary::RandomFloatInRange(0.f, max_weight);

        for (int i = 0; i < Loot_AthenaVendings_WithTier.size(); i++)
        {
            if (random_weight < Loot_AthenaVendings_WithTier[i]->Weight())
            {
                PickedLootTierData.push_back(Loot_AthenaVendings_WithTier[i]);

                Loot_AthenaVendings_WithTier.erase(Loot_AthenaVendings_WithTier.begin() + i);
            
                return true;
            }
            
            random_weight -= Loot_AthenaVendings_WithTier[i]->Weight();
        }

        return false;
    };

    // Set the vending machine's rarity
    vending_machine->StartingGoalLevel() = picked_tier;

    // Now set everything we got in the vending machine
    auto ItemCollections = TArrayManager<FCollectorUnitInfo>(&vending_machine->ItemCollections(), FCollectorUnitInfo::GetStructSize());

    for (int i = 0; i < ItemCollections.Num(); i++)
    {
        if (PickRandomLootTierData())
        {
            auto CurrentLoot = PickedLootTierData[i];

            if (CurrentLoot == nullptr)
            {
                continue;
            }

            if (wcsstr(CurrentLoot->LootPackage().ToString().c_str(), L"WorldList"))
            {
                std::vector<FFortLootPackageData*> possible_world_lists;
            
                for (auto WorldList : loot_tier_packages_map)
                {
                    FFortLootPackageData* category_loot_package_data = WorldList.Value();
        
                    if (category_loot_package_data->LootPackageID() == CurrentLoot->LootPackage())
                    {
                        if (category_loot_package_data->Weight() > 0.f)
                        {
                            possible_world_lists.push_back(category_loot_package_data);
                        }
                    }
                }

                float max_weight = 0.f;
            
                for (auto possible_world_list : possible_world_lists)
                {
                    max_weight += possible_world_list->Weight();
                }

                float random_weight = UKismetMathLibrary::RandomFloatInRange(0.f, max_weight);

                for (auto possible_world_list : possible_world_lists)
                {
                    if (random_weight < possible_world_list->Weight())
                    {
                        auto OutputItemEntries = TArrayManager<FFortItemEntry>(&ItemCollections[i].OutputItemEntry(), FFortItemEntry::GetStructSize());

                        OutputItemEntries.Add(&static_cast<UFortWorldItem*>(possible_world_list->ItemDefinition().Get()->CreateTemporaryItemInstanceBP(possible_world_list->Count(), 1))->ItemEntry());   
                    
                        break;
                    }

                    random_weight -= possible_world_list->Weight();
                }
            }
            else if (wcsstr(CurrentLoot->LootPackage().ToString().c_str(), L"WorldPKG"))
            {
                for (auto WorldPackage : loot_tier_packages_map)
                {
                    FFortLootPackageData* category_loot_package_data = WorldPackage.Value();
        
                    if (category_loot_package_data->LootPackageID() == CurrentLoot->LootPackage())
                    {
                        std::vector<FFortLootPackageData*> possible_world_lists;
                    
                        for (auto WorldList : loot_tier_packages_map)
                        {
                            FFortLootPackageData* world_list_package = WorldList.Value();

                            if (world_list_package->LootPackageID() == FName(category_loot_package_data->LootPackageCall().c_str()))
                            {
                                if (world_list_package->Weight() > 0.f)
                                {
                                    possible_world_lists.push_back(world_list_package);
                                }
                            }
                        }

                        float max_weight = 0.f;
                    
                        for (auto possible_world_list : possible_world_lists)
                        {
                            max_weight += possible_world_list->Weight();
                        }

                        float random_weight = UKismetMathLibrary::RandomFloatInRange(0.f, max_weight);

                        for (auto possible_world_list : possible_world_lists)
                        {
                            if (random_weight < possible_world_list->Weight())
                            {
                                auto OutputItemEntries = TArrayManager<FFortItemEntry>(&ItemCollections[i].OutputItemEntry(), FFortItemEntry::GetStructSize());
                        
                                OutputItemEntries.Add(&static_cast<UFortWorldItem*>(possible_world_list->ItemDefinition().Get()->CreateTemporaryItemInstanceBP(possible_world_list->Count(), 1))->ItemEntry());   
                                                    
                                break;
                            }

                            random_weight -= possible_world_list->Weight();
                        }
                    }
                }
            }

            // get the first item from the item entries and put it on display
            auto OutputItemEntries = TArrayManager<FFortItemEntry>(&ItemCollections[i].OutputItemEntry(), FFortItemEntry::GetStructSize());

            if(OutputItemEntries.Num() > 0)
            {
                ItemCollections[i].OutputItem() = static_cast<UFortWorldItemDefinition*>(OutputItemEntries[0].ItemDefinition());   
            }
        }
    }
}

FFortLootTierData* looting_component::get_random_loot_tier_data(FName loot_category)
{
    UObject* loot_tier_data = get_loot_tier_data();
    
    if (loot_tier_data == nullptr)
    {
        return nullptr;
    }

    TMap<FName, FFortLootTierData*> loot_tier_data_map = *reinterpret_cast<TMap<FName, FFortLootTierData*>*>(reinterpret_cast<uintptr_t>(loot_tier_data) + 0x30);

    // Row name and weight
    std::vector<FName> category_row_names;

    float max_weight = 0.0f;

    for (auto element : loot_tier_data_map)
    {
        // Category name
        FName category_row_name = element.Element.Key;

        // Loot tier data for the category
        FFortLootTierData* category_loot_tier_data = element.Element.Value;
        
        if (category_loot_tier_data->TierGroup() == loot_category)
        {
            if (category_loot_tier_data->Weight() > 0.0f)
            {
                category_row_names.push_back(category_row_name);
            }
        }
    }

    for (FName category_row_name : category_row_names)
    {
        // Increase max weight based on each individual category weight
        max_weight += loot_tier_data_map[category_row_name].Value->Weight();
    }

    FFortLootTierData* final_loot_tier_data = nullptr;

    // Generate random float between zero and the weight calculated
    float random_weight = UKismetMathLibrary::RandomFloatInRange(0.0f, max_weight);
    
    for (FName category_row_name : category_row_names)
    {
        if (random_weight < loot_tier_data_map[category_row_name].Value->Weight())
        {
            // Choose this row
            final_loot_tier_data = loot_tier_data_map[category_row_name].Value;

            break;
        }

        random_weight -= loot_tier_data_map[category_row_name].Value->Weight();
    }

    return final_loot_tier_data;
}

FFortLootPackageData* looting_component::get_random_loot_package_data(FFortLootTierData* loot_tier_data)
{
    UObject* loot_package_data = get_loot_tier_packages();
    
    if (loot_package_data == nullptr)
    {
        return nullptr;
    }

    TMap<FName, FFortLootPackageData*> loot_package_data_map = *reinterpret_cast<TMap<FName, FFortLootPackageData*>*>(reinterpret_cast<uintptr_t>(loot_package_data) + 0x30);

    // Row name and weight
    std::vector<FName> category_row_names;

    float max_weight = 0.0f;

    for (auto element : loot_package_data_map)
    {
        // Category name
        FName category_row_name = element.Element.Key;

        // Loot tier data for the category
        FFortLootPackageData* category_loot_tier_data = element.Element.Value;

        if(category_loot_tier_data && loot_tier_data)
        {
            if (category_loot_tier_data->LootPackageID() == loot_tier_data->LootPackage())
            {
                if (category_loot_tier_data->Weight() > 0.0f)
                {
                    category_row_names.push_back(category_row_name);
                }
            }   
        }
    }

    for (FName category_row_name : category_row_names)
    {
        // Increase max weight based on each individual category weight
        max_weight += loot_package_data_map[category_row_name].Value->Weight();
    }

    FFortLootPackageData* final_loot_package_data = nullptr;

    // Generate random float between zero and the weight calculated
    float random_weight = UKismetMathLibrary::RandomFloatInRange(0.0f, max_weight);
    
    for (FName category_row_name : category_row_names)
    {
        if (random_weight < loot_package_data_map[category_row_name].Value->Weight())
        {
            // Choose this row
            final_loot_package_data = loot_package_data_map[category_row_name].Value;

            break;
        }

        random_weight -= loot_package_data_map[category_row_name].Value->Weight();
    }

    return final_loot_package_data;
}   

UObject* looting_component::get_loot_tier_data()
{
    UObject* loot_tier_data = FindObject(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client", true);

    // Get current playlist 
    static bool b_has_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") || (*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo");
    
    if (b_has_playlist) // make sure there's a playlist property
    {
        UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
            (*GWorld)->GameState()->CurrentPlaylistData() : (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist();
    
        if (UObject* playlist_loot_tier_data = current_playlist->LootTierData().Get())
        {
            // Override loot tier data from playlist
            loot_tier_data = playlist_loot_tier_data;
        }
    }
    
    return loot_tier_data;
}

UObject* looting_component::get_loot_tier_packages()
{
    UObject* loot_tier_packages = FindObject(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client", true);

    // Get current playlist
    static bool b_has_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") || (*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo");
    
    if (b_has_playlist) // make sure there's a playlist property
    {
        UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
            (*GWorld)->GameState()->CurrentPlaylistData() : (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist();
    
        if (UObject* playlist_loot_tier_packages = current_playlist->LootPackages().Get())
        {
            // Override loot tier packages from playlist
            loot_tier_packages = playlist_loot_tier_packages;
        }
    }
    
    return loot_tier_packages;
}

int looting_component::get_default_loaded_ammo(UFortItemDefinition* in_item_definition)
{
    static auto FortWeaponItemDefinition = FindObject(L"/Script/FortniteGame.FortWeaponItemDefinition");
    static auto FortTrapItemDefinition = FindObject(L"/Script/FortniteGame.FortTrapItemDefinition");
    
    if (UKismetMathLibrary::ClassIsChildOf(in_item_definition->Class, FortWeaponItemDefinition) &&
                !UKismetMathLibrary::ClassIsChildOf(in_item_definition->Class, FortTrapItemDefinition))
    {
        auto Map = *reinterpret_cast<TMap<FName, FFortBaseWeaponStats*>*>(__int64(static_cast<UFortWeaponItemDefinition*>(in_item_definition)->WeaponStatHandle().Table) + 0x30);
        return Map[static_cast<UFortWeaponItemDefinition*>(in_item_definition)->WeaponStatHandle().RowName].Value->ClipSize();
    }
    
    return 0;
}

void looting_component_hooks::PickLootDrops_Hook(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName)
{
    if (OutLootToDrop == nullptr)
    {
        spdlog::error(L"looting_component_hooks::PickLootDrops_Hook failed: invalid OutLootToDrop");
    }

    std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(TierGroupName);

    for (const auto& [item_definition, count] : loot_to_spawn)
    {
        // Create a new item entry
        FFortItemEntry* NewItemEntry = &static_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(count, 1))->ItemEntry();

        // Add the newly created item entry to the OutLootToDrop array
        TArrayManager<FFortItemEntry> OutLootToDrop_ArrayManager = TArrayManager<FFortItemEntry>(OutLootToDrop, FFortItemEntry::GetStructSize());
        OutLootToDrop_ArrayManager.Add(NewItemEntry);
    }
}

void looting_component_hooks::PickLootDrops_New_Hook(UWorld*, TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName)
{
    PickLootDrops_Hook(OutLootToDrop, TierGroupName);   
}

AFortPickup* looting_component_hooks::AFortAthenaSupplyDrop_SpawnPickup_Hook(AFortAthenaSupplyDrop* FortAthenaSupplyDrop, UFortItemDefinition* ItemDefinition, int NumberToSpawn, AFortPawn* TriggeringPawn, FVector* Position)
{
    return pickup_component::spawn_pickup(ItemDefinition, *Position, NumberToSpawn, looting_component::get_default_loaded_ammo(ItemDefinition));
}

AFortGameModePickup* looting_component_hooks::AFortAthenaSupplyDrop_SpawnGameModePickup_Hook(AFortAthenaSupplyDrop* FortAthenaSupplyDrop, UFortItemDefinition* ItemDefinition, UObject* PickupClass, int NumberToSpawn, AFortPawn* TriggeringPawn, FVector* Position, FVector* Direction)
{
    return static_cast<AFortGameModePickup*>(pickup_component::spawn_pickup(ItemDefinition, *Position, NumberToSpawn, looting_component::get_default_loaded_ammo(ItemDefinition), PickupClass));
}   