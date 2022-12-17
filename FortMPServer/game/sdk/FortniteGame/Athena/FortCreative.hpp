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

    void OnPlayerPawnTeleported(AFortPlayerPawn* FortPlayerPawn)
    {
        STATIC_FINDFUNCTION(OnPlayerPawnTeleported)
        
        Call(Function_OnPlayerPawnTeleported, FortPlayerPawn);
    }

    void NotifyTeleportedPlayerPawn(AFortPlayerPawn* FortPlayerPawn)
    {
        STATIC_FINDFUNCTION(NotifyTeleportedPlayerPawn)
        
        Call(Function_NotifyTeleportedPlayerPawn, FortPlayerPawn);
    }
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
};

class ULevelStreaming;

class UFortPlaysetItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UWorld>, PlaysetToSpawn);

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

class UFortLevelSaveComponent : public UActorComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortLevelSaveComponent")
    
    DEFINE_PROPERTY(FUniqueNetIdRepl, AccountIdOfOwner);
    DEFINE_PROPERTY(FFortCreativePlotPermissionData, PlotPermissions);

    void OnRep_PlotPermissionsChanged()
    {
        STATIC_FINDFUNCTION(OnRep_PlotPermissionsChanged)
        
        Call(Function_OnRep_PlotPermissionsChanged);
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