#pragma once

class UAthenaGameData : public UObject
{
public:
    DEFINE_PROPERTY(TArray<UAthenaCharacterItemDefinition*>, RandomCharacters);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortHeroType>>, DefaultAthenaHeroes);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortItemDefinition>>, ShuffleTileItemDefinitions);
};

// Used for later builds (Most likely Chapter 2+)
class UGameDataCosmetics : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, EmoteGameplayAbility);
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, SprayGameplayAbility);
};

class UFortGameData : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, EmoteGameplayAbility);
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, SprayGameplayAbility);

    // USE FOR CHAPTER 1
    static UFortGameData* get_game_data()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        if (STATIC_HASPROPERTY(asset_manager, GameData))
        {
            auto GameData = asset_manager->GameData();
            return GameData;
        }
        return nullptr;
    }

    static UAthenaGameData* get_athena_game_data()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        if (STATIC_HASPROPERTY(asset_manager, AthenaGameData))
        {
            auto GameData = asset_manager->AthenaGameData();
            if (GameData == nullptr)
            {
                GameData = FindObject<UAthenaGameData>(asset_manager->AthenaGameDataName().c_str(), true, false);
            }
            return GameData;
        }
        return nullptr;
    }

    // USE FOR CHAPTER 2
    static UGameDataCosmetics* get_game_data_cosmetics()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        if (STATIC_HASPROPERTY(asset_manager, GameDataCosmetics))
        {
            auto GameData = asset_manager->GameDataCosmetics();
            return GameData;
        }
        return nullptr;
    }

    static UObject* get_emote_gameplay_ability()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        if (STATIC_HASPROPERTY(asset_manager, GameDataCosmetics))
        {
            auto GameData = asset_manager->GameDataCosmetics();
            return GameData->EmoteGameplayAbility().Get();
        }
        else
        {
            auto GameData = asset_manager->GameData();
            return GameData->EmoteGameplayAbility().Get();
        }
        
        return nullptr;
    }

    static UObject* get_spray_gameplay_ability()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        if (STATIC_HASPROPERTY(asset_manager, GameDataCosmetics))
        {
            auto GameData = asset_manager->GameDataCosmetics();
            return GameData->SprayGameplayAbility().Get();
        }
        else
        {
            auto GameData = asset_manager->GameData();
            return GameData->SprayGameplayAbility().Get();
        }
        
        return nullptr;
    }
};