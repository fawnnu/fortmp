#pragma once

class UFortPlaylist : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr, LootTierData);
    DEFINE_PROPERTY(TSoftObjectPtr, LootPackages);
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
};