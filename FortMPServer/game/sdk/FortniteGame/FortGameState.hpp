#pragma once
#include "Athena/MapInfo.hpp"
#include "sdk/FortniteGame/Athena/FortCreative.hpp"
#include "sdk/FortniteGame/Athena/FortPlaylist.hpp"
#include "sdk/Engine/Kismet/KismetLibrary.hpp"

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

struct FSpawnMachineRepData : FFastArraySerializerItem
{
    DEFINE_STRUCT_PROPERTY(FVector, Location);
    DEFINE_STRUCT_PROPERTY(uint8_t, SpawnMachineState);
    DEFINE_STRUCT_PROPERTY(float, SpawnMachineCooldownStartTime);
    DEFINE_STRUCT_PROPERTY(float, SpawnMachineCooldownEndTime);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameMemberInfo");
};

struct FActiveGameplayModifierHandle
{
public:
    int32                                        Handle;
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.ActiveGameplayModifierHandle");
};

struct FPlaylistPropertyArray
{
    DEFINE_STRUCT_PROPERTY(int, PlaylistReplicationKey);

    DEFINE_STRUCT_PROPERTY(class UFortPlaylistAthena*, BasePlaylist);
    
    DEFINE_STRUCT_PROPERTY(class UFortPlaylistAthena*, OverridePlaylist);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.PlaylistPropertyArray");
};

struct FSlateBrush
{
    DEFINE_STRUCT_CLASS(L"/Script/SlateCore.SlateBrush");
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
    
    DEFINE_PROPERTY(TArray<class AController*>, TeamMembers);
};

class AGameStateBase : public AActor
{
public:
    DEFINE_PROPERTY(UObject*, GameModeClass);
    
    DEFINE_PROPERTY(float, ReplicatedWorldTimeSeconds);

    void OnRep_GameModeClass()
    {
        STATIC_FINDFUNCTION(OnRep_GameModeClass)
        
        Call(Function_OnRep_GameModeClass);
    }
};

class AGameState : public AGameStateBase
{
public:
    DEFINE_PROPERTY(FName, MatchState);
};

struct FAdditionalLevelStreamed
{
    DEFINE_STRUCT_PROPERTY(FName, LevelName);
    DEFINE_STRUCT_PROPERTY(bool, bIsServerOnly);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AdditionalLevelStreamed");
};

class UBuildingStructuralSupportSystem : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingStructuralSupportSystem");

    bool IsBuildingActorStructurallySupported(class ABuildingSMActor* ActorToCheck)
    {
        STATIC_FINDFUNCTION(IsBuildingActorStructurallySupported)

        return Call<bool>(Function_IsBuildingActorStructurallySupported, ActorToCheck);
    }
};

class AFortGameState : public AGameState
{
public:
    DEFINE_PROPERTY(TArray<AFortTeamInfo*>, Teams);
    DEFINE_PROPERTY(TArray<char>, AdditionalPlaylistLevelsStreamed);

    DEFINE_PROPERTY(UBuildingStructuralSupportSystem*, StructuralSupportSystem);
    
    DEFINE_PROPERTY(AFortCreativePortalManager*, CreativePortalManager);

    bool IsRespawningAllowed(AFortPlayerState* PlayerState)
    {
        STATIC_FINDFUNCTION(IsRespawningAllowed)
        bool ret_val = Call<bool>(Function_IsRespawningAllowed, PlayerState);
        return ret_val;
    }
    
    void OnRep_AdditionalPlaylistLevelsStreamed()
    {
        STATIC_FINDFUNCTION(OnRep_AdditionalPlaylistLevelsStreamed)
        
        Call(Function_OnRep_AdditionalPlaylistLevelsStreamed);
    }

    void HandlePlaylistStreamedLevels(UFortPlaylistAthena* in_playlist)
    {
        // Loop through all the additional levels
        if (in_playlist == nullptr)
        {
            return;
        }

        // TODO: Fix the crash on s12 (levels are automatic on s13+)
        if (GetFortniteVersionEasy() > 12)
        {
            return;
        }
        
        if (STATIC_HASPROPERTY(in_playlist, AdditionalLevels))
        {
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
        }

        OnRep_AdditionalPlaylistLevelsStreamed();
    }
};

struct FGameMemberInfoArray : FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FGameMemberInfo>, Members);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameMemberInfoArray");
};

struct FSpawnMachineRepDataArray : FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FSpawnMachineRepData>, SpawnMachineRepDataArray);
    DEFINE_STRUCT_PROPERTY(AFortGameStateAthena*, OwningGameState);
    
    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameMemberInfoArray");
};

struct FSpecialActorSingleStatData
{
    DEFINE_STRUCT_PROPERTY(uint8, StatType);
    DEFINE_STRUCT_PROPERTY(float, Value);             
    DEFINE_STRUCT_PROPERTY(float, StatLogicValue);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.SpecialActorSingleStatData");
};

struct FSpecialActorRepData : public FFastArraySerializerItem
{
	DEFINE_STRUCT_PROPERTY(FName, SpecialActorID);              
	DEFINE_STRUCT_PROPERTY(struct FGameplayTag, CategoryTag);
	DEFINE_STRUCT_PROPERTY(class AActor*, SpecialActor);
    DEFINE_STRUCT_PROPERTY(bool, bActorIsRelevant);
    DEFINE_STRUCT_PROPERTY(FSlateBrush, MainIconBrush);
    DEFINE_STRUCT_PROPERTY(FSlateBrush, CompassIconBrush);
	DEFINE_STRUCT_PROPERTY(FVector, LastRepLocation);             
	DEFINE_STRUCT_PROPERTY(FVector, PrevRepLocation);             
	DEFINE_STRUCT_PROPERTY(FVector, LerpStartLocation);           
	DEFINE_STRUCT_PROPERTY(float, LastYawReplicationTime);      
	DEFINE_STRUCT_PROPERTY(float, PrevYawReplicatedTime);       
	DEFINE_STRUCT_PROPERTY(float, LastRepYaw);                  
	DEFINE_STRUCT_PROPERTY(float, PrevRepYaw);                  
	DEFINE_STRUCT_PROPERTY(float, LerpStartYaw);                
	DEFINE_STRUCT_PROPERTY(bool, bReplicateHealth);            
	DEFINE_STRUCT_PROPERTY(bool, bReplicateShield);
    DEFINE_STRUCT_PROPERTY(float, AddedToClientTime);
    DEFINE_STRUCT_PROPERTY(TArray<struct FSpecialActorSingleStatData>, TrackedStats);
    DEFINE_STRUCT_PROPERTY(float, LastLocationReplicationTime);
    DEFINE_STRUCT_PROPERTY(float, PrevLocationReplicatedTime);
    DEFINE_STRUCT_PROPERTY(struct FVector2D, MinimapScale);
    DEFINE_STRUCT_PROPERTY(struct FVector2D, CompassScale);

    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.SpecialActorRepData");
};

struct FAircraftFlightInfo
{
    DEFINE_STRUCT_PROPERTY(struct FVector, FlightStartLocation);
    DEFINE_STRUCT_PROPERTY(struct FRotator, FlightStartRotation);
    DEFINE_STRUCT_PROPERTY(float, FlightSpeed);        
    DEFINE_STRUCT_PROPERTY(float, TimeTillFlightEnd);  
    DEFINE_STRUCT_PROPERTY(float, TimeTillDropStart);  
    DEFINE_STRUCT_PROPERTY(float, TimeTillDropEnd);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AircraftFlightInfo");
};

struct FSpecialActorRepDataArray : FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FSpecialActorRepData>, SpecialActorList);

    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.SpecialActorRepDataArray");
};

class AFortSpecialActorReplicationInfo : public AActor
{
public:
    DEFINE_PROPERTY(struct FSpecialActorRepDataArray, SpecialActorRepList);
};

struct TeamsArrayContainer
{
    UE::TSparseArray<UE::TArray<TWeakObjectPtr<AFortPlayerStateAthena>>> TeamsArray{};
    UE::TSparseArray<UE::TArray<TWeakObjectPtr<AFortPlayerStateAthena>>> SquadsArray{};
};

class EAthenaGamePhase : public UEnum
{
public:
    DEFINE_STATIC_ENUM_CLASS(L"/Script/FortniteGame.EAthenaGamePhase")
    
    ENUM_PROPERTY(Aircraft)
};

class AFortGameStateAthena : public AFortGameState
{
public:
    DEFINE_STATIC_CLASS(TEXT("/Script/FortniteGame.FortGameStateAthena"));
    
    DEFINE_PROPERTY(TArray<UObject*>, BuildingActorClasses);
    DEFINE_PROPERTY(TArray<UObject*>, AllPlayerBuildableClasses);
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortGameStateAthena>, GamePhaseChanged);
    DEFINE_PROPERTY(TArray<AActor*>, Aircrafts);
    DEFINE_PROPERTY(AActor*, Aircraft);
    DEFINE_PROPERTY(FAircraftFlightInfo, FlightPathMidLine);
    DEFINE_PROPERTY(FVector2D, FlightPathSelectionCenter);
    DEFINE_PROPERTY(UFortItemDefinition*, DefaultBattleBus);
    DEFINE_PROPERTY(AFortWorldManager*, WorldManager);
    DEFINE_PROPERTY(class AFortSafeZoneIndicator*, SafeZoneIndicator);
    DEFINE_PROPERTY(float, DefaultAllowNeutralWallEditing);
    DEFINE_PROPERTY(float, DefaultRebootMachineHotfix);
    DEFINE_PROPERTY(uint8_t, GamePhase);
    DEFINE_PROPERTY(uint8_t, AirCraftBehavior);
    DEFINE_PROPERTY(bool, bIsLargeTeamGame);
    DEFINE_PROPERTY(bool, bPlaylistDataIsLoaded);
    
    DEFINE_PROPERTY(UFortPlaylistAthena*, CurrentPlaylistData);
    DEFINE_PROPERTY(int, CurrentPlaylistId);
    DEFINE_PROPERTY(FPlaylistPropertyArray, CurrentPlaylistInfo);

    DEFINE_PROPERTY(FGameMemberInfoArray, GameMemberInfoArray);
    DEFINE_PROPERTY(FSpawnMachineRepDataArray, SpawnMachineRepData);

    DEFINE_PROPERTY(int, TotalPlayers);
    DEFINE_PROPERTY(int, PlayersLeft);
    DEFINE_PROPERTY(int, TeamsLeft);

    DEFINE_PROPERTY(FVector, FloatingIslandPosition);
    
    DEFINE_PROPERTY(class AFortAthenaMapInfo*, MapInfo);

    DEFINE_PROPERTY(class AFortVolumeManager*, VolumeManager);

    DEFINE_PROPERTY(class AFortSpecialActorReplicationInfo*, SpecialActorData);

    DEFINE_PROPERTY(uint8_t, FriendlyFireType);
    
    TeamsArrayContainer* GetTeamsArrayContainer()
    {
        static void* native_property = FindPropertyByName(this->Class, L"FriendlyFireType");                                          
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   

        return (TeamsArrayContainer*)(__int64(this) + (native_property_offset + 0x5));
    }
    
    void ChangeBattleBusCosmetic(UFortItemDefinition* BusCosmetic)
    {
        if (STATIC_HASPROPERTY(this, DefaultBattleBus))
        {
            if (UFortItemDefinition* BattleBusOverride = BusCosmetic)
            {
                DefaultBattleBus() = BattleBusOverride;
            }
        }
    }

    void ChangeSupplyDropCosmetic(UFortItemDefinition* SupplyDropCosmetic)
    {
        if (auto supply_drop_info = FindObject<UFortSupplyDropInfo>(L"/Game/Athena/Balance/MapInfos/DefaultSupplyDrop.DefaultSupplyDrop", true, false))
        {
            supply_drop_info->SupplyDropClass() = SupplyDropCosmetic;
        }
    }
    
    // Custom event
    void WaitForMapInfo()
    {
        STATIC_FINDFUNCTION(WaitForMapInfo)
        
        Call(Function_WaitForMapInfo);
    }
    
    void WaitForServerReady()
    {
        STATIC_FINDFUNCTION(WaitForServerReady)
        
        Call(Function_WaitForServerReady);
    }
    
    void OnRep_CurrentPlaylistInfo()
    {
        STATIC_FINDFUNCTION(OnRep_CurrentPlaylistInfo)
        
        Call(Function_OnRep_CurrentPlaylistInfo);
    }

    FActiveGameplayModifierHandle RegisterGameplayModifier(UObject* InModifierToRegister, int32 Expiration)
    {
        STATIC_FINDFUNCTION(RegisterGameplayModifier)
        
        return Call<FActiveGameplayModifierHandle>(Function_RegisterGameplayModifier, InModifierToRegister, Expiration);
    }
    
    void SetFloatingIslandInformation(struct FVector& WorldPosition, bool bActivated)
    {
        STATIC_FINDFUNCTION(SetFloatingIslandInformation)
        
        Call(Function_SetFloatingIslandInformation, WorldPosition, bActivated);
    }
    
    void OnRep_CurrentPlaylistData()
    {
        STATIC_FINDFUNCTION(OnRep_CurrentPlaylistData)
        
        Call(Function_OnRep_CurrentPlaylistData);
    }
};