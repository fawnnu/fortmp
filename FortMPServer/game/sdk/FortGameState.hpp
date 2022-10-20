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

struct FAdditionalLevelStreamed
{
    DEFINE_STRUCT_PROPERTY(FName, LevelName);
    DEFINE_STRUCT_PROPERTY(bool, bIsServerOnly);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AdditionalLevelStreamed");
};

class AFortGameState : public AGameStateBase
{
public:
    DEFINE_PROPERTY(TArray<AFortTeamInfo*>, Teams);
    DEFINE_PROPERTY(TArray<char>, AdditionalPlaylistLevelsStreamed);

    void OnRep_AdditionalPlaylistLevelsStreamed()
    {
        Call(L"OnRep_AdditionalPlaylistLevelsStreamed");
    }

    void HandlePlaylistStreamedLevels(UFortPlaylistAthena* in_playlist)
    {
        // Loop through all the additional levels
        for (auto streamed_world : in_playlist->AdditionalLevels())
        {
            // Get object from Soft Object Pointer
            if (UObject* streamed_world_object = streamed_world.Get())
            {
                // Get the path name
                FString streamed_world_path = UKismetSystemLibrary::GetPathName(streamed_world_object);

                TArrayManager<char> AdditionalPlaylistLevelsStreamed_ArrayManager = TArrayManager<char>(&AdditionalPlaylistLevelsStreamed(), FAdditionalLevelStreamed::GetClass() ? FAdditionalLevelStreamed::GetStructSize() : sizeof FName);

                if (FAdditionalLevelStreamed::GetClass())
                {
                    FAdditionalLevelStreamed* NewAdditionalLevelStreamed = static_cast<FAdditionalLevelStreamed*>(malloc(FAdditionalLevelStreamed::GetStructSize()));

                    ZeroMemory(NewAdditionalLevelStreamed, FAdditionalLevelStreamed::GetStructSize());

                    NewAdditionalLevelStreamed->LevelName() = streamed_world_path.c_str();

                    AdditionalPlaylistLevelsStreamed_ArrayManager.Add(&NewAdditionalLevelStreamed);

                    spdlog::info(L"Loaded Additional playlist level: '{}'", NewAdditionalLevelStreamed->LevelName().ToString().c_str());
                }
                else
                {
                    FName LevelName = streamed_world_path.c_str();

                    AdditionalPlaylistLevelsStreamed_ArrayManager.Add(&LevelName);

                    spdlog::info(L"Loaded Additional playlist level: '{}'", LevelName.ToString().c_str());
                }
            }
            else
            {
                spdlog::error(L"Invalid AdditionalLevel when executing HandlePlaylistStreamedLevels");
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

    DEFINE_PROPERTY(class AFortVolumeManager*, VolumeManager);
    
    void OnRep_CurrentPlaylistInfo()
    {
        Call(L"OnRep_CurrentPlaylistInfo");
    }

    void OnRep_CurrentPlaylistData()
    {
        Call(L"OnRep_CurrentPlaylistData");
    }
};