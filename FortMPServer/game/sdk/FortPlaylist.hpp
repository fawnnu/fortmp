#pragma once

class UFortPlaylist : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr, LootTierData);
    DEFINE_PROPERTY(TSoftObjectPtr, LootPackages);
    DEFINE_PROPERTY(TSoftObjectPtr, GameData);

    DEFINE_PROPERTY(int, MaxTeamSize);
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
};