#pragma once

class UAthenaGameData : public UObject
{
public:
    DEFINE_PROPERTY(TArray<UAthenaCharacterItemDefinition*>, RandomCharacters);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortHeroType>>, DefaultAthenaHeroes);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortItemDefinition>>, ShuffleTileItemDefinitions);
};

class UFortGameData : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, EmoteGameplayAbility);
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, SprayGameplayAbility);
    
    static UFortGameData* get_game_data()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        auto GameData = asset_manager->GameData();
        return GameData;
    }

    static UAthenaGameData* get_athena_game_data()
    {
        UFortAssetManager* asset_manager = static_cast<UFortAssetManager*>((*GEngine)->AssetManager());
        if (!asset_manager) return nullptr;

        auto GameData = asset_manager->AthenaGameData();
        if (GameData == nullptr)
        {
            GameData = FindObject<UAthenaGameData>(asset_manager->AthenaGameDataName().c_str(), true, false);
        }
        
        return GameData;
    }
};