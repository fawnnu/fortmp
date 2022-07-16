#pragma once

struct FCreateBuildingActorData
{
    DEFINE_STRUCT_PROPERTY(uint32_t, BuildingClassHandle);
    DEFINE_STRUCT_PROPERTY(FVector, BuildLoc);
    DEFINE_STRUCT_PROPERTY(FRotator, BuildRot);
    DEFINE_STRUCT_PROPERTY(bool, bMirrored);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.CreateBuildingActorData");
};

struct FBuildingClassData
{
    DEFINE_STRUCT_PROPERTY(UObject*, BuildingClass);
    DEFINE_STRUCT_PROPERTY(int, PreviousBuildingLevel);
    DEFINE_STRUCT_PROPERTY(int, UpgradeLevel);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.BuildingClassData");
};

class ABuildingActor : public AActor
{
public:
    DEFINE_PROPERTY(uint8_t, Team);

    void InitializeKismetSpawnedBuildingActor(ABuildingActor* BuildingOwner, AFortPlayerController* SpawningController)
    {
        static bool has_spawning_anim = FindObject(L"/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor")->HasProperty(L"bUsePlayerBuildAnimations");
        
        // Spawn building actor depending on property
        has_spawning_anim ?
            Call(L"InitializeKismetSpawnedBuildingActor", BuildingOwner, SpawningController, true) : Call(L"InitializeKismetSpawnedBuildingActor", BuildingOwner, SpawningController);
    }

    float GetHealth()
    {
        return Call<float>(L"GetHealth");
    }

    float GetMaxHealth()
    {
        return Call<float>(L"GetMaxHealth");
    }
};

class ABuildingSMActor : public ABuildingActor
{
public:
    DEFINE_PROPERTY(uint8_t, AttributeInitLevelSource);
    
    DEFINE_PROPERTY(struct FCurveTableRowHandle, BuildingResourceAmountOverride);
    
    DEFINE_PROPERTY(uint8_t, ResourceType);
    
    void SetMirrored(bool bMirrored)
    {
        Call(L"SetMirrored", bMirrored);
    }

    void RepairBuilding(AFortPlayerController* RepairingController, int ResourcesSpent)
    {
        Call(L"RepairBuilding", RepairingController, ResourcesSpent);
    }
};

class AFortWeap_EditingTool : public UObject
{
public:
    DEFINE_PROPERTY(ABuildingSMActor*, EditActor);
};

class ABuildingContainer : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(FName, SearchLootTierGroup);
    DEFINE_PROPERTY(FVector, LootSpawnLocation_Athena);

    DEFINE_BIT_PROPERTY(bool, bAlreadySearched);

    void OnRep_bAlreadySearched()
    {
        Call(L"OnRep_bAlreadySearched");
    }
};

struct FRebootCardReplicatedState
{
    DEFINE_STRUCT_PROPERTY(float, ChipExpirationServerStartTime);
    DEFINE_STRUCT_PROPERTY(AFortPlayerStateAthena*, PlayerState);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.RebootCardReplicatedState");
};

class ABuildingGameplayActorSpawnChip : public ABuildingActor
{
public:
    DEFINE_PROPERTY(AFortPlayerController*, OwnerPlayerController);
    DEFINE_PROPERTY(AFortPlayerStateAthena*, OwnerPlayerState);
    DEFINE_PROPERTY(uint8_t, OwnerTeam);
    DEFINE_PROPERTY(FUniqueNetIdRepl, OwnerPlayerId);

    DEFINE_PROPERTY(FRebootCardReplicatedState, RebootCardReplicatedState);
};

class ABuildingGameplayActorSpawnMachine : public ABuildingActor
{
public:
    DEFINE_PROPERTY(AActor*, ResurrectLocation);
    DEFINE_PROPERTY(TArray<FUniqueNetIdRepl>, PlayerIdsForResurrection);

    void SetSpawnMachineState(uint8_t NewState)
    {
        Call(L"SetSpawnMachineState", NewState);
    }
};