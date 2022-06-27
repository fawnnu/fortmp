#pragma once

struct FLevelCollection
{
    unsigned char             UnknownData00[0x8];             
    class UObject*            GameState;                             
    class UNetDriver*         NetDriver;                      
    class UObject*            DemoNetDriver;                         
    class UObject*            PersistentLevel;               
    unsigned char             UnknownData01[0x50];            
    unsigned char             UnknownData02[0x8];             
};

class UWorld : public UObject
{
public:
    DEFINE_PROPERTY(AFortGameStateAthena*, GameState);
    DEFINE_PROPERTY(AFortGameModeAthena*, AuthorityGameMode);

    DEFINE_PROPERTY(UNetDriver*, NetDriver);

    DEFINE_PROPERTY(TArray<FLevelCollection>, LevelCollections);
};

class ULevelStreaming : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr, WorldAsset);
    DEFINE_PROPERTY(FName, PackageNameToLoad);
};