#pragma once

struct FFortLevelStreamingInfo
{
    struct FName PackageName;
    uint8_t LevelState;
    char pad2[7];
};

struct FPlaylistPropertyArray
{
    DEFINE_STRUCT_PROPERTY(int, PlaylistReplicationKey);

    DEFINE_STRUCT_PROPERTY(class UFortPlaylistAthena*, BasePlaylist);
    
    DEFINE_STRUCT_PROPERTY(class UFortPlaylistAthena*, OverridePlaylist);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.PlaylistPropertyArray");
};

class AFortWorldManager : public UObject
{
public:
    DEFINE_PROPERTY(TArray<FFortLevelStreamingInfo>, StreamedLevels);
};

class AFortGameStateAthena : public UObject
{
public:
    DEFINE_PROPERTY(TArray<UObject*>, BuildingActorClasses);

    DEFINE_PROPERTY(TArray<AActor*>, Aircrafts);

    DEFINE_PROPERTY(AFortWorldManager*, WorldManager);

    DEFINE_PROPERTY(uint8_t, GamePhase);

    DEFINE_PROPERTY(UObject*, CurrentPlaylistData);

    DEFINE_PROPERTY(FPlaylistPropertyArray, CurrentPlaylistInfo);
    
    void OnRep_CurrentPlaylistInfo()
    {
        Call(L"OnRep_CurrentPlaylistInfo");
    }

    void OnRep_CurrentPlaylistData()
    {
        Call(L"OnRep_CurrentPlaylistData");
    }
};