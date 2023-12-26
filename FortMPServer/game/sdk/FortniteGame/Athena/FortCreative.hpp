#pragma once
#include "sdk/Engine/Components/ActorComponent.hpp"

struct FFortPlaysetStreamingData
{
    DEFINE_STRUCT_PROPERTY(FName, PackageName);
    DEFINE_STRUCT_PROPERTY(FName, UniquePackageName);
    DEFINE_STRUCT_PROPERTY(FVector, Location);
    DEFINE_STRUCT_PROPERTY(FRotator, Rotation);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPlaysetStreamingData");
};

struct FFortCreativePlotPermissionData
{
    DEFINE_STRUCT_PROPERTY(uint8, Permission);
    DEFINE_STRUCT_PROPERTY(TArray<struct FString>, WhiteList);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortCreativePlotPermissionData");
};

class AFortVolume;

struct FMinigamePlayer : public FFastArraySerializerItem
{
public:
    DEFINE_STRUCT_PROPERTY(AActor*, ClaimedPlayerStart);
    DEFINE_STRUCT_PROPERTY(AFortPlayerState*, PlayerState);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.MinigamePlayer")
};

struct FMinigamePlayers : public FFastArraySerializer
{
public:
    DEFINE_STRUCT_PROPERTY(TArray<FMinigamePlayer>*, Players);
    DEFINE_STRUCT_PROPERTY(class AFortMinigame*, Owner);
	
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.MinigamePlayers")
};

class AFortMinigame : public AActor
{
public:
    
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortMinigame");

    DEFINE_PROPERTY(AFortVolume*, Volume);
    DEFINE_PROPERTY(FMinigamePlayers, PlayerArray);
    DEFINE_PROPERTY(uint8, CurrentState);

    AActor* ChoosePlayerStart(AController* Player)
    {
        STATIC_FINDFUNCTION(ChoosePlayerStart)

        return Call<AActor*>(Function_ChoosePlayerStart, Player);
    }

    bool DetermineSpawnLocation(class AFortPlayerStateAthena* PlayerState, struct FVector* OutLocation, struct FRotator* OutRotation, bool* bOutRespawningOnGround)
    {
        STATIC_FINDFUNCTION(DetermineSpawnLocation)

        struct 
        {
            AFortPlayerStateAthena* PlayerState;
            FVector OutLocation;
            FRotator OutRotation;
            bool bOutRespawningOnGround;
            bool ReturnValue;
        } parameters;

        parameters.PlayerState = PlayerState;
        
        ProcessEvent(this, Function_DetermineSpawnLocation, &parameters);

        if (OutLocation)
        {
            *OutLocation = parameters.OutLocation;
        }
        if (OutRotation)
        {
            *OutRotation = parameters.OutRotation;
        }
        if (bOutRespawningOnGround)
        {
            *bOutRespawningOnGround = parameters.bOutRespawningOnGround;
        }
        
        return parameters.ReturnValue;
    }
};

class AFortAthenaCreativePortal : public AActor
{
public:
    DEFINE_PROPERTY(int, PortalIndex);
    DEFINE_PROPERTY(AFortVolume*, LinkedVolume);
    DEFINE_PROPERTY(FUniqueNetIdRepl, OwningPlayer);
    DEFINE_PROPERTY(bool, bPortalOpen);
    DEFINE_PROPERTY(AFortPlayerState*, CachedOwningPlayerState);
    DEFINE_PROPERTY(bool, bDisallowPortalInteract);
    DEFINE_PROPERTY(FString, ImageUrl);
    DEFINE_PROPERTY(FVector, TeleportLocation);
    DEFINE_PROPERTY(TArray<struct FUniqueNetIdRepl>, PlayersReady);
    DEFINE_PROPERTY(uint8, CurrentPopulation);
    DEFINE_PROPERTY(FText, TeleportingPlayersText);
    
    void OnRep_PortalOpen()
    {
        STATIC_FINDFUNCTION(OnRep_PortalOpen)
        
        Call(Function_OnRep_PortalOpen);
    }

    void OnRep_OwningPlayer()
    {
        STATIC_FINDFUNCTION(OnRep_OwningPlayer)
        
        Call(Function_OnRep_OwningPlayer);
    }

    void OnRep_PlayersReady()
    {
        STATIC_FINDFUNCTION(OnRep_PlayersReady)
        
        Call(Function_OnRep_PlayersReady);
    }
    
    void OwningPlayerChanged()
    {
        STATIC_FINDFUNCTION(OwningPlayerChanged)
        
        Call(Function_OwningPlayerChanged);
    }

    void PortalOpenChanged()
    {
        STATIC_FINDFUNCTION(PortalOpenChanged)
        
        Call(Function_PortalOpenChanged);
    }

    void OnPlayerPawnTeleported(class AFortPlayerPawn* FortPlayerPawn)
    {
        STATIC_FINDFUNCTION(OnPlayerPawnTeleported)
        
        Call(Function_OnPlayerPawnTeleported, FortPlayerPawn);
    }

    void NotifyTeleportedPlayerPawn(class AFortPlayerPawn* FortPlayerPawn)
    {
        STATIC_FINDFUNCTION(NotifyTeleportedPlayerPawn)
        
        Call(Function_NotifyTeleportedPlayerPawn, FortPlayerPawn);
    }
    bool ClientSaveStateChanged(bool bSaving)
    {
        STATIC_FINDFUNCTION(ClientSaveStateChanged)
        
        return Call<bool>(Function_ClientSaveStateChanged, bSaving);
    }
    bool ClientSaveDisplayChanged(bool bSaving)
    {
        STATIC_FINDFUNCTION(ClientSaveDisplayChanged)
        
        return Call<bool>(Function_ClientSaveDisplayChanged, bSaving);
    }
};

class AFortPlayerStartCreative : public AActor
{
public:
    DEFINE_PROPERTY(int, PortalIndex);
};

class AFortCreativePortalManager : public AActor
{
public:
    DEFINE_PROPERTY(TArray<class AFortAthenaCreativePortal*>, AllPortals);
    DEFINE_PROPERTY(TArray<class AFortAthenaCreativePortal*>, UsedPortals);
    DEFINE_PROPERTY(TArray<class AFortAthenaCreativePortal*>, AvailablePortals);

    void MarkPortalUsed(AFortAthenaCreativePortal* PortalUsed)
    {
        STATIC_FINDFUNCTION(MarkPortalUsed)
        
        Call(Function_MarkPortalUsed, PortalUsed);
    }
    
    void ReleasePortal(AFortAthenaCreativePortal* PortalUsed)
    {
        STATIC_FINDFUNCTION(ReleasePortal)
        
        Call(Function_ReleasePortal, PortalUsed);
    }
};

struct FBaseActorRecord
{
    
};

struct FNewActorRecord
{
    
};

struct FDeleteActorRecord
{
    
};

class ULevelSaveRecord : public UObject
{
public:
    DEFINE_PROPERTY(TArray<FBaseActorRecord>, BaseActorRecords);
    DEFINE_PROPERTY(TArray<FNewActorRecord>, NewActorRecords);
    DEFINE_PROPERTY(TArray<FDeleteActorRecord>, DeleteActorRecords);

    void PostUpdateSpawnedActorsHelper(int StartIndex)
    {
        STATIC_FINDFUNCTION(PostUpdateSpawnedActorsHelper)

        Call(Function_PostUpdateSpawnedActorsHelper, StartIndex);
    }
    
    void FindAndRemoveDuplicateRecordsUsingBucketMap(int StartIndex)
    {
        STATIC_FINDFUNCTION(FindAndRemoveDuplicateRecordsUsingBucketMap)

        Call(Function_FindAndRemoveDuplicateRecordsUsingBucketMap,StartIndex);
    }
    
    void NotifyFinishedSpawning()
    {
        STATIC_FINDFUNCTION(NotifyFinishedSpawning)

        Call(Function_NotifyFinishedSpawning);
    }
};

class ULevelStreaming;

class UFortPlaysetItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UWorld>, PlaysetToSpawn);
    DEFINE_PROPERTY(ULevelSaveRecord*, LevelSaveRecord);
    
    ULevelStreaming* SpawnPlayset(UWorld* WorldContextObject, UFortItemDefinition* Playset, FVector Location, FRotator Rotation, FFortPlaysetStreamingData* OutLevelData)
    {
        struct SpawnPlayset_Params
        {
            DEFINE_STRUCT_PROPERTY(UWorld*, WorldContextObject);
            DEFINE_STRUCT_PROPERTY(UFortItemDefinition*, Playset);
            DEFINE_STRUCT_PROPERTY(FVector, Location);
            DEFINE_STRUCT_PROPERTY(FRotator, Rotation);
            DEFINE_STRUCT_PROPERTY(FFortPlaysetStreamingData, OutLevelData);
            DEFINE_STRUCT_PROPERTY(ULevelStreaming*, ReturnValue);
        
            DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPlaysetItemDefinition.SpawnPlayset");
        };

        SpawnPlayset_Params* Params = static_cast<SpawnPlayset_Params*>(alloca(SpawnPlayset_Params::GetStructSize()));
    
        ZeroMemory(Params, SpawnPlayset_Params::GetStructSize());

        Params->WorldContextObject() = WorldContextObject;
        Params->Playset() = Playset;
        Params->Location() = Location;
        Params->Rotation() = Rotation;

        ProcessEvent(this, SpawnPlayset_Params::GetClass(), Params);

        memcpy(OutLevelData, &Params->OutLevelData(), FFortPlaysetStreamingData::GetStructSize());

        return Params->ReturnValue();
    }
};

class ULevelSaveComponent : public UActorComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.LevelSaveComponent")

    DEFINE_PROPERTY(ULevelSaveRecord*, LevelRecord);

    void ApplySaveToWorld(bool bComplexShouldSpawn)
    {
        STATIC_FINDFUNCTION(ApplySaveToWorld)

        Call(Function_ApplySaveToWorld, bComplexShouldSpawn);
    }

    void LoadLevelRecord()
    {
        STATIC_FINDFUNCTION(LoadLevelRecord)

        Call(Function_LoadLevelRecord);
    }
};

class UFortLevelSaveComponent : public ULevelSaveComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortLevelSaveComponent")
    
    DEFINE_PROPERTY(FUniqueNetIdRepl, AccountIdOfOwner);
    DEFINE_PROPERTY(FFortCreativePlotPermissionData, PlotPermissions);
    DEFINE_PROPERTY(bool, bIsLoaded);
    DEFINE_PROPERTY(UObject*, LoadedPlot);

    void OnRep_PlotPermissionsChanged()
    {
        STATIC_FINDFUNCTION(OnRep_PlotPermissionsChanged)
        
        Call(Function_OnRep_PlotPermissionsChanged);
    }
    void OnRep_LoadedPlotInstanceId()
    {
        STATIC_FINDFUNCTION(OnRep_LoadedPlotInstanceId)
        
        Call(Function_OnRep_LoadedPlotInstanceId);
    }
    void OnRep_Loaded()
    {
        STATIC_FINDFUNCTION(OnRep_Loaded)
        
        Call(Function_OnRep_Loaded);
    }
    void ApplySaveToWorld(bool bComplexShouldSpawn)
    {
        STATIC_FINDFUNCTION(ApplySaveToWorld)
        
        Call(Function_ApplySaveToWorld, bComplexShouldSpawn);
    }
    void ConstructNewLevelRecord(AActor* Owner)
    {
        STATIC_FINDFUNCTION(ConstructNewLevelRecord)
        
        Call(Function_ConstructNewLevelRecord, Owner);
    }
    void PostUpdateSpawnedActorsHelper(int StartingIndex)
    {
        STATIC_FINDFUNCTION(PostUpdateSpawnedActorsHelper)
        
        Call(Function_PostUpdateSpawnedActorsHelper, StartingIndex);
    }
    void BroadcastPostUpdateSpawnedCompleted()
    {
        STATIC_FINDFUNCTION(BroadcastPostUpdateSpawnedCompleted)
        
        Call(Function_BroadcastPostUpdateSpawnedCompleted);
    }
    void NotifyFinishedSpawning()
    {
        STATIC_FINDFUNCTION(NotifyFinishedSpawning)
        
        Call(Function_NotifyFinishedSpawning);
    }
};

class UPlaysetLevelStreamComponent : public UActorComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.PlaysetLevelStreamComponent")
    
    DEFINE_BIT_PROPERTY(uint8, bAutoLoadLevel);
    DEFINE_PROPERTY(UFortPlaysetItemDefinition*, CurrentPlayset);
    DEFINE_PROPERTY(FFortPlaysetStreamingData, ClientPlaysetData);
    DEFINE_PROPERTY(ULevelStreaming*, StreamedLevel);
    DEFINE_PROPERTY(TArray<struct FUniqueNetIdRepl>, PlayersWithValidPlaysetData);
    DEFINE_PROPERTY(FName, ClientLoadedPackageName);

    void SetPlayset(UFortPlaysetItemDefinition* NewPlayset)
    {
        STATIC_FINDFUNCTION(SetPlayset)
        
        Call(Function_SetPlayset, NewPlayset);
    }

    void OnRep_ClientPlaysetData()
    {
        STATIC_FINDFUNCTION(OnRep_ClientPlaysetData)
        
        Call(Function_OnRep_ClientPlaysetData);
    }

    void OnFinishedLoading()
    {
        STATIC_FINDFUNCTION(OnFinishedLoading)
        
        Call(Function_OnFinishedLoading);
    }

    void OnFinishedLoading_Internal()
    {
        STATIC_FINDFUNCTION(OnFinishedLoading_Internal)
        
        Call(Function_OnFinishedLoading_Internal);
    }
};

class ABGA_IslandPortal_C : public AFortAthenaCreativePortal
{
public:
    DEFINE_CLASS(L"/Game/Playgrounds/Items/BGA_IslandPortal.BGA_IslandPortal_C")
};