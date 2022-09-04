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

struct FCollectorUnitInfo
{
    DEFINE_STRUCT_PROPERTY(UFortWorldItemDefinition*, InputItem);
    DEFINE_STRUCT_PROPERTY(UFortWorldItemDefinition*, OutputItem);

    DEFINE_STRUCT_PROPERTY(TArray<FFortItemEntry>, OutputItemEntry);
    
    static UObject* GetClass()                                                                         
    {                                                                                                  
        static UObject* class_ptr = FindObject(L"/Script/FortniteGame.CollectorUnitInfo") ?
            FindObject(L"/Script/FortniteGame.CollectorUnitInfo") :
            FindObject(L"/Script/FortniteGame.ColletorUnitInfo");                                             
       
        return class_ptr;                                                                              
    }                                                                                                  
    
    static int GetStructSize()                                                                         
    {                                                                                                  
        return *reinterpret_cast<int*>(__int64(GetClass()) + CoreOffsets::StructSize);                 
    }
};

class ABuildingActor : public AActor
{
public:
    DEFINE_PROPERTY(uint8_t, Team);

    void InitializeKismetSpawnedBuildingActor(ABuildingActor* BuildingOwner, AFortPlayerController* SpawningController)
    {
        static bool has_spawning_anim_arg = FindPropertyByName(FindObject(L"/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor"), L"bUsePlayerBuildAnimations") != nullptr;
        
        // Spawn building actor depending on property
        has_spawning_anim_arg ?
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

    void SetTeam(uint8_t TeamIndex)
    {
        Call(L"SetTeam", TeamIndex);
    }
};

class ABuildingSMActor : public ABuildingActor
{
public:
    DEFINE_PROPERTY(uint8_t, AttributeInitLevelSource);
    
    DEFINE_PROPERTY(struct FCurveTableRowHandle, BuildingResourceAmountOverride);
    
    DEFINE_PROPERTY(uint8_t, ResourceType);

    DEFINE_PROPERTY(TArray<ABuildingSMActor*>, AttachedBuildingActors);
    
    void SetMirrored(bool bMirrored)
    {
        Call(L"SetMirrored", bMirrored);
    }

    void RepairBuilding(AFortPlayerController* RepairingController, int ResourcesSpent)
    {
        Call(L"RepairBuilding", RepairingController, ResourcesSpent);
    }
};

class ABuildingItemCollectorActor : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(int, StartingGoalLevel);

    DEFINE_PROPERTY(TArray<FCollectorUnitInfo>, ItemCollections);
};

class AB_Athena_VendingMachine_C : public ABuildingItemCollectorActor
{
public:
    DEFINE_PROPERTY(uint8_t, OverrideVendingMachineRarity);
    
    void SetRarityColors(FColor Color)
    {
        Call(L"SetRarityColors", Color);
    }
};

class AFortWeap_EditingTool : public UObject
{
public:
    DEFINE_PROPERTY(ABuildingSMActor*, EditActor);
};


class AFortWeap_BuildingTool : public AActor
{
public:
    DEFINE_PROPERTY(class UBuildingEditModeMetadata*, DefaultMetadata);

    void OnRep_DefaultMetadata()
    {
        Call(L"OnRep_DefaultMetadata");
    }
};

class UBuildingEditModeMetadata : public UObject
{
public:
};

class ABuildingContainer : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(FName, SearchLootTierGroup);
    DEFINE_PROPERTY(FVector, LootSpawnLocation_Athena);

    DEFINE_BIT_PROPERTY(bool, bAlreadySearched);
    DEFINE_BIT_PROPERTY(bool, bStartAlreadySearched_Athena);

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