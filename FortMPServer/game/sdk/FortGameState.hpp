#pragma once

struct FFortLevelStreamingInfo
{
    struct FName PackageName;
    uint8_t LevelState;
    char pad2[7];
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

    void OnRep_CurrentPlaylistData()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistData");
        
        // TODO: use Call
        ProcessEvent(this, function, nullptr);
    }
};