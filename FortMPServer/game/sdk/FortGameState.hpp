#pragma once

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

    DEFINE_PROPERTY(UFortPlaylistAthena*, CurrentPlaylistData);

    DEFINE_PROPERTY(FPlaylistPropertyArray, CurrentPlaylistInfo);

    DEFINE_PROPERTY(FGameMemberInfoArray, GameMemberInfoArray);

    DEFINE_PROPERTY(int, TotalPlayers);
    DEFINE_PROPERTY(int, PlayersLeft);
    
    void OnRep_CurrentPlaylistInfo()
    {
        Call(L"OnRep_CurrentPlaylistInfo");
    }

    void OnRep_CurrentPlaylistData()
    {
        Call(L"OnRep_CurrentPlaylistData");
    }
};