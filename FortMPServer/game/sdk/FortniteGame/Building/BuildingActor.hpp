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

struct FFortSearchBounceData
{
    DEFINE_STRUCT_PROPERTY(FVector, BounceNormal);
    DEFINE_STRUCT_PROPERTY(uint32, SearchAnimationCount);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortSearchBounceData");
};

struct FCollectorUnitInfo
{
    DEFINE_STRUCT_PROPERTY(UFortWorldItemDefinition*, InputItem);
    DEFINE_STRUCT_PROPERTY(UFortWorldItemDefinition*, OutputItem);

    DEFINE_STRUCT_PROPERTY(TArray<FFortItemEntry>, OutputItemEntry);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, InputCount);
    
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
    DEFINE_PROPERTY(float, VertSnapGridSize);

    void InitializeKismetSpawnedBuildingActor(ABuildingActor* BuildingOwner, AFortPlayerController* SpawningController)
    {
        static bool has_spawning_anim_arg = FindPropertyByName(FindObject(L"/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor"), L"bUsePlayerBuildAnimations") != nullptr;

        STATIC_FINDFUNCTION(InitializeKismetSpawnedBuildingActor)
        
        // Spawn building actor depending on property
        has_spawning_anim_arg ?
            Call(Function_InitializeKismetSpawnedBuildingActor, BuildingOwner, SpawningController, true) : Call(Function_InitializeKismetSpawnedBuildingActor, BuildingOwner, SpawningController);
    }

    float GetHealth()
    {
        STATIC_FINDFUNCTION(GetHealth)
        
        return Call<float>(Function_GetHealth);
    }

    float GetMaxHealth()
    {
        STATIC_FINDFUNCTION(GetMaxHealth)
        
        return Call<float>(Function_GetMaxHealth);
    }

    void SetTeam(uint8_t TeamIndex)
    {
        STATIC_FINDFUNCTION(SetTeam)
        
        Call(Function_SetTeam, TeamIndex);
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
        STATIC_FINDFUNCTION(SetMirrored)
        
        Call(Function_SetMirrored, bMirrored);
    }

    void RepairBuilding(AFortPlayerController* RepairingController, int ResourcesSpent)
    {
        STATIC_FINDFUNCTION(RepairBuilding)
        
        Call(Function_RepairBuilding, RepairingController, ResourcesSpent);
    }
};

class ABuildingItemCollectorActor : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(int, StartingGoalLevel);

    DEFINE_PROPERTY(TArray<FCollectorUnitInfo>, ItemCollections);

    DEFINE_PROPERTY(UFortItemDefinition*, ActiveInputItem);

    DEFINE_PROPERTY(FVector, LootSpawnLocation);
    DEFINE_PROPERTY(FRotator, LootTossDirection);
    DEFINE_PROPERTY(float, LootTossConeHalfAngle);
    DEFINE_PROPERTY(float, LootTossSpeed);

    DEFINE_PROPERTY(int, NumInteractionsAllowed);
    DEFINE_PROPERTY(int, NumInteractions);

    int GetDepositAmount(AFortPlayerController* FortPlayerController, UFortItemDefinition* InInputItem)
    {
        STATIC_FINDFUNCTION(GetDepositAmount)
        
        return Call<int>(Function_GetDepositAmount, FortPlayerController, InInputItem);
    }
    
    int GetDepositGoal(AFortPlayerController* FortPlayerController, UFortItemDefinition* InInputItem)
    {
        STATIC_FINDFUNCTION(GetDepositGoal)
        
        return Call<int>(Function_GetDepositGoal, FortPlayerController, InInputItem);
    }
};

class AB_Athena_VendingMachine_C : public ABuildingItemCollectorActor
{
public:
    DEFINE_PROPERTY(uint8_t, OverrideVendingMachineRarity);
    
    void SetRarityColors(FColor Color)
    {
        STATIC_FINDFUNCTION(SetRarityColors)
        
        Call(Function_SetRarityColors, Color);
    }

    void DoVendDeath()
    {
        STATIC_FINDFUNCTION(DoVendDeath)

        Call(Function_DoVendDeath);
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
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeap_BuildingTool")
    
    DEFINE_PROPERTY(class UBuildingEditModeMetadata*, DefaultMetadata);

    void OnRep_DefaultMetadata()
    {
        STATIC_FINDFUNCTION(OnRep_DefaultMetadata)
        
        Call(Function_OnRep_DefaultMetadata);
    }
};

class UBuildingEditModeMetadata : public UObject
{
public:
};

class ABuildingContainer : public ABuildingSMActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingContainer");
    DEFINE_PROPERTY(FName, SearchLootTierGroup);
    DEFINE_PROPERTY(FVector, LootSpawnLocation_Athena);
    DEFINE_PROPERTY(FVector, LootSpawnLocation);
    DEFINE_PROPERTY(FFortSearchBounceData, SearchBounceData);
    
    DEFINE_BIT_PROPERTY(bool, bAlreadySearched);
    DEFINE_BIT_PROPERTY(bool, bStartAlreadySearched_Athena);

    void OnRep_bAlreadySearched()
    {
        STATIC_FINDFUNCTION(OnRep_bAlreadySearched)
        
        Call(Function_OnRep_bAlreadySearched);
    }

    FVector CalculateLootSpawnLocation()
    {
        FVector ForwardVector = GetActorRightVector();
        FVector LootAdjustmentVector;

        if (STATIC_HASPROPERTY(this, LootSpawnLocation_Athena))
        {
            LootAdjustmentVector = LootSpawnLocation_Athena();
        }
        else
        {
            LootAdjustmentVector = LootSpawnLocation();
        }

        FVector right_vector = UKismetMathLibrary::GetRightVector(K2_GetActorRotation());
        right_vector.X *= LootAdjustmentVector.X;
        right_vector.Z *= LootAdjustmentVector.Y;

        FVector UpVector = GetActorUpVector();
        UpVector.X *= std::clamp(LootAdjustmentVector.X, 1.f, 999.f);
        UpVector.Y *= std::clamp(LootAdjustmentVector.Y, 1.f, 999.f);
        UpVector.Z *= std::clamp(LootAdjustmentVector.Z, 1.f, 999.f);

        right_vector.X += UpVector.X;
        right_vector.Y += UpVector.Y;
        right_vector.Z += UpVector.Z;

        FVector FinalLocation = this->K2_GetActorLocation();
        FinalLocation.X += right_vector.X;
        FinalLocation.Y += right_vector.Y;
        FinalLocation.Z += right_vector.Z;

        return FinalLocation;
    }
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingFoundation");

    DEFINE_PROPERTY(uint8_t, DynamicFoundationType);
};

struct FRebootCardReplicatedState
{
    DEFINE_STRUCT_PROPERTY(float, ChipExpirationServerStartTime);
    DEFINE_STRUCT_PROPERTY(AFortPlayerStateAthena*, PlayerState);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.RebootCardReplicatedState");
};

class AFortPhysicsBall : public ABuildingActor
{
};

class AB_PlayerSpawnedBall_C : public AFortPhysicsBall
{
public:
    void InitializeToyInstance(AFortPlayerController* player_controller, int NumTimesSummoned)
    {
        STATIC_FINDFUNCTION(InitializeToyInstance)

        Call(Function_InitializeToyInstance, player_controller, NumTimesSummoned);
    }
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
        STATIC_FINDFUNCTION(SetSpawnMachineState)
        
        Call(Function_SetSpawnMachineState, NewState);
    }
};

class ABGAConsumableSpawner : public ABuildingActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BGAConsumableSpawner")
    
    DEFINE_PROPERTY(FName, SpawnLootTierGroup);
    DEFINE_PROPERTY(UEnvQuery*, QueryTemplate);
    DEFINE_PROPERTY(TArray<FFortItemEntry>, ConsumablesToSpawn);
};

class ACBGA_LowGravity_UsingJump_C : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/ForagedItems/LowGravity/CBGA_LowGravity_UsingJump.CBGA_LowGravity_UsingJump_C");
};

class ACBGA_HealthSmall_C : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/ForagedItems/HealthSmall/CBGA_HealthSmall.CBGA_HealthSmall_C");
};

class ACBGA_ShieldsSmall_C : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/ForagedItems/ShieldsSmall/CBGA_ShieldsSmall.CBGA_ShieldsSmall_C");
};