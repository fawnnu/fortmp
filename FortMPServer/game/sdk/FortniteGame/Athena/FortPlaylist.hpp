#pragma once

class UFortPlaylist : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootTierData);
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootPackages);
    DEFINE_PROPERTY(TSoftObjectPtr<class UCurveTable>, GameData);
    DEFINE_PROPERTY(TSoftObjectPtr<class UCurveTable>, ResourceRates);
    DEFINE_PROPERTY(uint8_t, DefaultFirstTeam);
    DEFINE_PROPERTY(uint8_t, DefaultLastTeam);
    DEFINE_PROPERTY(int, MaxTeamSize);
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlaylistAthena");
    DEFINE_PROPERTY(struct FScalableFloat, RespawnHeight);

    DEFINE_PROPERTY(uint8_t, RespawnType);
    DEFINE_PROPERTY(uint8_t, AirCraftBehavior);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<class UObject>>, AdditionalLevels);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<class UObject>>, AdditionalLevelsServerOnly);
    DEFINE_BIT_PROPERTY(bool, bEnableCreativeMode);
};