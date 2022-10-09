#pragma once
#include "FortPlaylist.hpp"
#include "KismetLibrary.hpp"

struct FFortLevelStreamingInfo
{
    struct FName PackageName;
    uint8_t LevelState;
    char pad2[7];

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortLevelStreamingInfo");
};

struct FGameMemberInfo : FFastArraySerializerItem
{
    uint8_t SquadId;
    uint8_t TeamIndex;
    char UnknownData00[0x2];
    FUniqueNetIdRepl MemberUniqueId;

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameMemberInfo");
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

class AFortTeamInfo : public UObject
{
public:
    DEFINE_PROPERTY(uint8_t, Team);
    
    DEFINE_PROPERTY(TArray<AController*>, TeamMembers);
};

class AGameStateBase : public AActor
{
public:
    DEFINE_PROPERTY(float, ReplicatedWorldTimeSeconds);
};

class AFortGameState : public AGameStateBase
{
public:
    DEFINE_PROPERTY(TArray<AFortTeamInfo*>, Teams);
    DEFINE_PROPERTY(TArray<struct FName>, AdditionalPlaylistLevelsStreamed);

    void OnRep_AdditionalPlaylistLevelsStreamed()
    {
        Call(L"OnRep_AdditionalPlaylistLevelsStreamed");
    }

    void AddPlaylistStreamedLevel(FName PackageName)
    {
        // Add a level
        AdditionalPlaylistLevelsStreamed().Add(PackageName);

        // Let the server know the levels were updated
        OnRep_AdditionalPlaylistLevelsStreamed();
    }

    void HandlePlaylistStreamedLevels(UFortPlaylistAthena* in_playlist)
    {
        for(auto streamed_world : in_playlist->AdditionalLevels())
        {
            UObject* streamed_world_object = streamed_world.Get();
            
            if(streamed_world_object != nullptr)
            {
                FString streamed_world_path = UKismetSystemLibrary::GetPathName(streamed_world_object);
                AdditionalPlaylistLevelsStreamed().Add(streamed_world_path.c_str());
                spdlog::info(L"Loading playlist level: {}", streamed_world_path.c_str());
            }
        }

        OnRep_AdditionalPlaylistLevelsStreamed();
    }
};

struct FGameMemberInfoArray : FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FGameMemberInfo>, Members);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameMemberInfoArray");
};

class AFortGameStateAthena : public AFortGameState
{
public:
    DEFINE_PROPERTY(TArray<UObject*>, BuildingActorClasses);
    DEFINE_PROPERTY(TArray<UObject*>, AllPlayerBuildableClasses);

    DEFINE_PROPERTY(TArray<AActor*>, Aircrafts);
    DEFINE_PROPERTY(AActor*, Aircraft);

    DEFINE_PROPERTY(AFortWorldManager*, WorldManager);

    DEFINE_PROPERTY(uint8_t, GamePhase);
    DEFINE_PROPERTY(uint8_t, AirCraftBehavior);
    
    DEFINE_PROPERTY(UFortPlaylistAthena*, CurrentPlaylistData);

    DEFINE_PROPERTY(FPlaylistPropertyArray, CurrentPlaylistInfo);

    DEFINE_PROPERTY(FGameMemberInfoArray, GameMemberInfoArray);

    DEFINE_PROPERTY(UObject*, DefaultBattleBus);
    
    DEFINE_PROPERTY(int, TotalPlayers);
    DEFINE_PROPERTY(int, PlayersLeft);

    DEFINE_PROPERTY(class AFortAthenaMapInfo*, MapInfo);
    
    void OnRep_CurrentPlaylistInfo()
    {
        Call(L"OnRep_CurrentPlaylistInfo");
    }

    void OnRep_CurrentPlaylistData()
    {
        Call(L"OnRep_CurrentPlaylistData");
    }
};