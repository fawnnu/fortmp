#include "include.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "inventory_component.hpp"

#include <random>

#include "reviving_component.hpp"

looting_component::looting_component()
{
    blacklisted_item_definitions = {};
    blacklisted_classes = {FindObject(L"/Script/FortniteGame.FortGadgetItemDefinition", true)};
}

std::map<UFortItemDefinition*, int> looting_component::spawn_loot(FString loot_category)
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
        
        if (category_loot_tier_data->TierGroup().ToString() == loot_category.c_str())
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
