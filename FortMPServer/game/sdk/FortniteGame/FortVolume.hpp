#pragma once

class AFortVolume : public AActor
{
public:

    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortVolume");
    
    DEFINE_PROPERTY(TArray<class AFortAthenaCreativePortal*>, LinkedPortals);
    DEFINE_PROPERTY(class UFortPlaysetItemDefinition*, CurrentPlayset);
    DEFINE_PROPERTY(UFortPlaysetItemDefinition*, OverridePlayset);
    DEFINE_PROPERTY(uint8, VolumeState);

    void SetCurrentPlayset(UFortPlaysetItemDefinition* NewPlayset)
    {
        STATIC_FINDFUNCTION(SetCurrentPlayset)
        
        Call(Function_SetCurrentPlayset, NewPlayset);
    }

    void OnRep_VolumeState()
    {
        STATIC_FINDFUNCTION(OnRep_VolumeState)
        
        Call(Function_OnRep_VolumeState);
    }
    
    void OnRep_CurrentPlayset()
    {
        STATIC_FINDFUNCTION(OnRep_CurrentPlayset);
        
        Call(Function_OnRep_CurrentPlayset);
    }
};

class AFortVolumeManager : public AActor
{
public:
    DEFINE_PROPERTY(TArray<AFortVolume*>, VolumeObjects);

    void UpdateVolumeLevelStreams()
    {
        STATIC_FINDFUNCTION(UpdateVolumeLevelStreams)
        
        Call(Function_UpdateVolumeLevelStreams);
    }
    
    void ResetVolumeAndLevelRecord(class AFortVolume* Volume, class AFortPlayerController* RequestingFortPC)
    {
        STATIC_FINDFUNCTION(ResetVolumeAndLevelRecord)
        
        Call(Function_ResetVolumeAndLevelRecord, Volume, RequestingFortPC);
    }
    
    void MinigameExitReadOnlyForVolume(AFortVolume* Volume)
    {
        STATIC_FINDFUNCTION(MinigameExitReadOnlyForVolume)
        
        Call(Function_MinigameExitReadOnlyForVolume, Volume);
    }
    
    void ApplyLevelSaveToWorld(class ULevelSaveRecord* SaveToSpawnFrom, bool bComplexShouldSpawn)
    {
        STATIC_FINDFUNCTION(ApplyLevelSaveToWorld)
        
        Call(Function_ApplyLevelSaveToWorld,SaveToSpawnFrom, bComplexShouldSpawn);
    }

    AFortVolume* SpawnVolume(UObject* VolumeClass, UFortPlaysetItemDefinition* Playset, FVector Location, FRotator Rotation)
    {
        STATIC_FINDFUNCTION(SpawnVolume);

        return Call<AFortVolume*>(Function_SpawnVolume, VolumeClass, Playset, Location, Rotation);
    }

    AFortMinigame* GetMinigameForVolume(AFortVolume* Volume)
    {
        STATIC_FINDFUNCTION(GetMinigameForVolume);

        return Call<AFortMinigame*>(Function_GetMinigameForVolume, Volume);
    }
};

struct FEraPoiInfo
{
    bool bLoggedInvalidBrushComponent;
};

class AFortPoiVolume : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, BrushComponent);
    DEFINE_PROPERTY(FEraPoiInfo, EraPoiInfo);
};

class AFortPoiManager : public AActor
{
public:
    
};