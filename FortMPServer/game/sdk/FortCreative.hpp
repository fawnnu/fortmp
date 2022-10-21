#pragma once
#include "ActorComponent.hpp"

struct FFortPlaysetStreamingData
{
    DEFINE_STRUCT_PROPERTY(FName, PackageName);
    DEFINE_STRUCT_PROPERTY(FName, UniquePackageName);
    DEFINE_STRUCT_PROPERTY(FVector, Location);
    DEFINE_STRUCT_PROPERTY(FRotator, Rotation);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPlaysetStreamingData");
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
        Call(L"OnRep_PortalOpen");
    }

    void OnRep_OwningPlayer()
    {
        Call(L"OnRep_OwningPlayer");
    }

    void OnRep_PlayersReady()
    {
        Call(L"OnRep_PlayersReady");
    }
    
    void OwningPlayerChanged()
    {
        Call(L"OwningPlayerChanged");
    }

    void PortalOpenChanged()
    {
        Call(L"PortalOpenChanged");
    }

    void OnPlayerPawnTeleported(AFortPlayerPawn* FortPlayerPawn, bool bTeleportedToIslandStart)
    {
        Call(L"OnPlayerPawnTeleported", FortPlayerPawn, bTeleportedToIslandStart);
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
        Call(L"MarkPortalUsed", PortalUsed);
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

class UPlaysetLevelStreamComponent : public UActorComponent
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.PlaysetLevelStreamComponent");

        return static_class;
    }
    
    DEFINE_BIT_PROPERTY(uint8, bAutoLoadLevel);
    DEFINE_PROPERTY(UFortPlaysetItemDefinition*, CurrentPlayset);
    DEFINE_PROPERTY(FFortPlaysetStreamingData, ClientPlaysetData);
    DEFINE_PROPERTY(ULevelStreaming*, StreamedLevel);
    DEFINE_PROPERTY(TArray<struct FUniqueNetIdRepl>, PlayersWithValidPlaysetData);
    DEFINE_PROPERTY(FName, ClientLoadedPackageName);

    void SetPlayset(UFortPlaysetItemDefinition* NewPlayset)
    {
        Call(L"SetPlayset", NewPlayset);
    }

    void OnRep_ClientPlaysetData()
    {
        Call(L"OnRep_ClientPlaysetData");
    }

    void OnFinishedLoading()
    {
        Call(L"OnFinishedLoading");
    }

    void OnFinishedLoading_Internal()
    {
        Call(L"OnFinishedLoading_Internal");
    }
};