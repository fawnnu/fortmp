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

class UGameInstance : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.GameInstance");
    
    DEFINE_PROPERTY(TArray<ULocalPlayer*>, LocalPlayers);
};

class UFortGameInstance : public UGameInstance
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameInstance");
};

class UAssetManager : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AssetManager");
};

class UFortAssetManager : public UAssetManager
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAssetManager");
    
    DEFINE_PROPERTY(class UFortGameData*, GameData);
    DEFINE_PROPERTY(class UGameDataCosmetics*, GameDataCosmetics);
    DEFINE_PROPERTY(class UAthenaGameData*, AthenaGameData);
    DEFINE_PROPERTY(FString, AthenaGameDataName);
};

class UEngine : public UObject
{
public:
    DEFINE_PROPERTY(UAssetManager*, AssetManager);
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

    DEFINE_PROPERTY(UGameInstance*, OwningGameInstance);
    
    AWorldSettings* K2_GetWorldSettings()
    {
        STATIC_FINDFUNCTION(K2_GetWorldSettings)
        
        return Call<AWorldSettings*>(Function_K2_GetWorldSettings);
    }
};

class ULevelStreaming : public UObject
{
public:
    DEFINE_PROPERTY(FName, PackageNameToLoad);
};