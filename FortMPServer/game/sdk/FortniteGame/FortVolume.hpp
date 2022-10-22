#pragma once

class AFortVolume : public AActor
{
public:
    DEFINE_PROPERTY(TArray<class AFortAthenaCreativePortal*>, LinkedPortals);
    DEFINE_PROPERTY(class UFortPlaysetItemDefinition*, CurrentPlayset);

    void SetCurrentPlayset(UFortPlaysetItemDefinition* NewPlayset)
    {
        Call(L"SetCurrentPlayset", NewPlayset);
    }

    void OnRep_CurrentPlayset()
    {
        Call(L"OnRep_CurrentPlayset");
    }
};

class AFortVolumeManager : public AActor
{
public:
    DEFINE_PROPERTY(TArray<AFortVolume*>, VolumeObjects);
};

class AFortPoiVolume : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, BrushComponent);
};
