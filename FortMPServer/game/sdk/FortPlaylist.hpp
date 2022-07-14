#pragma once

class UFortPlaylist : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootTierData);
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootPackages);
    DEFINE_PROPERTY(TSoftObjectPtr<class UCurveTable>, GameData);

    DEFINE_PROPERTY(int, MaxTeamSize);
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
};