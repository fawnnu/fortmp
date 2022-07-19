#pragma once

struct FNetViewer
{
    UNetConnection* Connection;
    AActor* InViewer;
    AActor* ViewTarget;
    FVector ViewLocation;
    FVector ViewDir;        
};

struct FLevelCollection
{
    unsigned char             UnknownData00[0x8];             
    class UObject*            GameState;                             
    class UNetDriver*         NetDriver;                      
    class UObject*            DemoNetDriver;                         
    class UObject*            PersistentLevel;               
    unsigned char             UnknownData01[0x50];         

    DEFINE_STRUCT_CLASS(L"/Script/Engine.LevelCollection");
};

class AWorldSettings : public UObject
{
public:
    DEFINE_PROPERTY(TArray<FNetViewer>, ReplicationViewers);
};

class ULevel : public UObject
{
public:
    DEFINE_PROPERTY(AWorldSettings*, WorldSettings);
};

class UWorld : public UObject
{
public:
    DEFINE_PROPERTY(AFortGameStateAthena*, GameState);
    DEFINE_PROPERTY(AFortGameModeAthena*, AuthorityGameMode);

    DEFINE_PROPERTY(UNetDriver*, NetDriver);

    DEFINE_PROPERTY(TArray<FLevelCollection>, LevelCollections);

    DEFINE_PROPERTY(ULevel*, PersistentLevel);

    DEFINE_PROPERTY_BY_OFFSET(float, TimeSeconds, NetworkOffsets::TimeSeconds);
    
    AWorldSettings* K2_GetWorldSettings()
    {
        return Call<AWorldSettings*>(L"K2_GetWorldSettings");
    }
};

class ULevelStreaming : public UObject
{
public:
    DEFINE_PROPERTY(FName, PackageNameToLoad);
};