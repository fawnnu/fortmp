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

enum class EFortBuildPreviewMarkerOptionalAdjustment : uint8_t
{
    None = 0,
    FreeWallPieceOnTop = 1,
    FreeWallPieceOnBottom = 2
};

class ABuildingActor : public AActor
{
public:
    DEFINE_PROPERTY(uint8_t, Team);
    DEFINE_PROPERTY(float, VertSnapGridSize);
    DEFINE_PROPERTY(FGameplayTagContainer, StaticGameplayTags);

    void InitializeKismetSpawnedBuildingActor(ABuildingActor* BuildingOwner, AFortPlayerController* SpawningController)
    {
        static bool has_spawning_anim_arg = FindPropertyByName(FindObject(L"/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor"), L"bUsePlayerBuildAnimations") != nullptr;

        STATIC_FINDFUNCTION(InitializeKismetSpawnedBuildingActor)
        
        // Spawn building actor depending on property
        has_spawning_anim_arg ?
            Call(Function_InitializeKismetSpawnedBuildingActor, BuildingOwner, SpawningController, true) : Call(Function_InitializeKismetSpawnedBuildingActor, BuildingOwner, SpawningController);
    }

    void SilentDie()
    {
        STATIC_FINDFUNCTION(SilentDie)

        Call(Function_SilentDie);
    }
    
    void OnRep_Team()
    {
        STATIC_FINDFUNCTION(OnRep_Team)
        
        Call(Function_OnRep_Team);
    }

    void PlacedByPlacementTool()
    {
        STATIC_FINDFUNCTION(PlacedByPlacementTool)
        
        Call(Function_PlacedByPlacementTool);
    }

    void OnWorldReady()
    {
        STATIC_FINDFUNCTION(OnWorldReady)
        
        Call(Function_OnWorldReady);
    }

    void PlacedByPlayer(AFortPlayerState* PlayerState)
    {
        STATIC_FINDFUNCTION(PlacedByPlayer)
        
        Call(Function_PlacedByPlayer, PlayerState);
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

    AController* GetOwningController()
    {
        STATIC_FINDFUNCTION(GetOwningController)

        return Call<AController*>(Function_GetOwningController);
    }

    AFortPlayerState* GetOwningPlayerState()
    {
        STATIC_FINDFUNCTION(GetOwningPlayerState)

        return Call<AFortPlayerState*>(Function_GetOwningPlayerState);
    }
};

enum class EFortBuildingType : uint8_t
{
    Wall                           = 0,
    Floor                          = 1,
    Corner                         = 2,
    Deco                           = 3,
    Prop                           = 4,
    Stairs                         = 5,
    Roof                           = 6,
    Pillar                         = 7,
    SpawnedItem                    = 8,
    Container                      = 9,
    Trap                           = 10,
    GenericCenterCellActor         = 11,
    None                           = 12
};

class ABuildingSMActor : public ABuildingActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingSMActor")
    DEFINE_PROPERTY(uint8_t, AttributeInitLevelSource);

    DEFINE_PROPERTY(int, OwnerPersistentID);
    
    DEFINE_PROPERTY(struct FCurveTableRowHandle, BuildingResourceAmountOverride);
    
    DEFINE_PROPERTY(uint8_t, ResourceType);
    DEFINE_PROPERTY(EFortBuildingType, BuildingType)
    
    DEFINE_PROPERTY(int, MaxResourcesToSpawn);
    DEFINE_PROPERTY(int, CurrentBuildingLevel);
    DEFINE_PROPERTY(TArray<ABuildingSMActor*>, AttachedBuildingActors);
    DEFINE_PROPERTY(TArray<ABuildingSMActor*>, BuildingActorsAttachedTo);
    DEFINE_PROPERTY(ABuildingSMActor*, DamagerOwner);
    
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
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingItemCollectorActor")
    DEFINE_PROPERTY(int, StartingGoalLevel);

    DEFINE_PROPERTY(TArray<FCollectorUnitInfo>, ItemCollections);

    DEFINE_PROPERTY(UFortItemDefinition*, ActiveInputItem);

    DEFINE_PROPERTY(FVector, LootSpawnLocation);
    DEFINE_PROPERTY(FRotator, LootTossDirection);
    DEFINE_PROPERTY(float, LootTossConeHalfAngle);
    DEFINE_PROPERTY(float, LootTossSpeed);
    DEFINE_PROPERTY(float, DespensingDelayTime);
    DEFINE_PROPERTY(float, OutputDespenseDelayTime);
    DEFINE_PROPERTY(int, NumInteractionsAllowed);
    DEFINE_PROPERTY(int, NumInteractions);

    DEFINE_BIT_PROPERTY(bool, bCurrentInteractionSuccess);

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
    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C")
    
    DEFINE_PROPERTY(uint8_t, OverrideVendingMachineRarity);
    DEFINE_PROPERTY(bool, VendSuccess);
    DEFINE_PROPERTY(int, MaxItemsToSpawnCached);
    DEFINE_PROPERTY(int, MaxItemsToSpawn);
    DEFINE_PROPERTY(int, SpawnedItemCount);
    
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
    DEFINE_PROPERTY(FRotator, LootTossDirection_Athena);
    DEFINE_PROPERTY(float, LootTossSpeed_Athena);
    DEFINE_PROPERTY(float, LootTossConeHalfAngle_Athena);
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
        FVector ActorRightVector = GetActorRightVector();
        FVector ActorUpVector = GetActorUpVector();
        FVector LootAdjustmentVector;

        if (STATIC_HASPROPERTY(this, LootSpawnLocation_Athena))
        {
            LootAdjustmentVector = LootSpawnLocation_Athena();
        }
        else
        {
            LootAdjustmentVector = LootSpawnLocation();
        }

        FVector FinalLocation = K2_GetActorLocation();
        ActorUpVector = ActorUpVector * LootAdjustmentVector.Z;
        ActorRightVector = ActorRightVector * LootAdjustmentVector.Y;
        
        FinalLocation = FinalLocation + ActorUpVector;
        FinalLocation = FinalLocation + ActorRightVector;

        return FinalLocation;
    }
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingFoundation");

    DEFINE_PROPERTY(uint8_t, DynamicFoundationType);
    DEFINE_PROPERTY(uint8_t, FoundationEnabledState);
    DEFINE_PROPERTY(uint8, bConditionalFoundation);
    DEFINE_PROPERTY(FName, LevelToStream);
    DEFINE_BIT_PROPERTY(bool, bFoundationEnabled);
    
    void OnRep_ServerStreamedInLevel()
    {
        STATIC_FINDFUNCTION(OnRep_ServerStreamedInLevel)

        Call(Function_OnRep_ServerStreamedInLevel);
    }
    
    void OnRep_LevelToStream()
    {
        STATIC_FINDFUNCTION(OnRep_LevelToStream)

        Call(Function_OnRep_LevelToStream);
    }
    
    void OnLevelStreamedIn()
    {
        STATIC_FINDFUNCTION(OnLevelStreamedIn)

        Call(Function_OnLevelStreamedIn);
    }
    void OnLevelShown()
    {
        STATIC_FINDFUNCTION(OnLevelShown)

        Call(Function_OnLevelShown);
    }
    void SetDynamicFoundationEnabled(bool bEnabled)
    {
        STATIC_FINDFUNCTION(SetDynamicFoundationEnabled)

        Call(Function_SetDynamicFoundationEnabled, bEnabled);
    }

    void OnRep_FoundationEnabled(bool bEnabled)
    {
        STATIC_FINDFUNCTION(OnRep_FoundationEnabled)

        if (Function_OnRep_FoundationEnabled)
        {
            if (FindPropertyByName(Function_OnRep_FoundationEnabled, TEXT("bEnabled")))
            {
                Call(Function_OnRep_FoundationEnabled, bEnabled);
            }
            else
            {
                Call(Function_OnRep_FoundationEnabled);
            }
        }
    }
    void OnRep_DynamicFoundationTransform()
    {
        STATIC_FINDFUNCTION(OnRep_DynamicFoundationTransform)

        Call(Function_OnRep_DynamicFoundationTransform);
    }
    void DisableReadyForPostUpdate()
    {
        if (auto Property = FindPropertyByName(Class, TEXT("bFoundationEnabled")))
        {
            uint8* ByteValue = reinterpret_cast<uint8*>(__int64(this) + *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(Property) + CoreOffsets::Offset_Internal));
            constexpr uint8 Mask = 0x8;
            *ByteValue = *ByteValue & ~Mask;
        }
    }
    void EnableReadyForPostUpdate()
    {
        if (auto Property = FindPropertyByName(Class, TEXT("bFoundationEnabled")))
        {
            uint8* ByteValue = reinterpret_cast<uint8*>(__int64(this) + *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(Property) + CoreOffsets::Offset_Internal));
            constexpr uint8 Mask = 0x8;
            *ByteValue = *ByteValue & ~Mask | Mask;
        }
    }
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
    DEFINE_PROPERTY(AFortPlayerPawn*, OwningPawn);
    
    void InitializeToyInstance(AFortPlayerController* player_controller, int NumTimesSummoned)
    {
        STATIC_FINDFUNCTION(InitializeToyInstance)

        Call(Function_InitializeToyInstance, player_controller, NumTimesSummoned);
    }

    void StartToyFadeOutDueToNewPlacement()
    {
        STATIC_FINDFUNCTION(StartToyFadeOutDueToNewPlacement)

        Call(Function_StartToyFadeOutDueToNewPlacement);
    }
};

class ABuildingGameplayActorSpawnChip : public ABuildingActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingGameplayActorSpawnChip")
    
    DEFINE_PROPERTY(AFortPlayerController*, OwnerPlayerController);
    DEFINE_PROPERTY(AFortPlayerStateAthena*, OwnerPlayerState);
    DEFINE_PROPERTY(uint8_t, OwnerTeam);
    DEFINE_PROPERTY(uint8_t, SquadId);
    DEFINE_PROPERTY(int, UnHide);
    DEFINE_PROPERTY(bool, bAutoAcquireSpawnChip);
    DEFINE_PROPERTY(FUniqueNetIdRepl, OwnerPlayerId);

    DEFINE_PROPERTY(FRebootCardReplicatedState, RebootCardReplicatedState);

    void OnRep_RebootCardReplicatedState()
    {
        STATIC_FINDFUNCTION(OnRep_RebootCardReplicatedState)
        
        Call(Function_OnRep_RebootCardReplicatedState);
    }
};

class ABuildingGameplayActorSpawnMachine : public ABuildingActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BuildingGameplayActorSpawnMachine")
    
    DEFINE_PROPERTY(AActor*, ResurrectLocation);
    DEFINE_PROPERTY(TArray<FUniqueNetIdRepl>, PlayerIdsForResurrection);
    DEFINE_PROPERTY(FMarkerID, ResurrectionChipMarkerID);
    DEFINE_PROPERTY(TWeakObjectPtr<AFortPlayerControllerAthena>, InstigatorPC);
    DEFINE_PROPERTY(uint8_t, SquadId);

    bool RebootPlayersInAir()
    {
        STATIC_FINDFUNCTION(RebootPlayersInAir)

        return Call<bool>(Function_RebootPlayersInAir);
    }
    
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
    DEFINE_CLASS(L"/Game/Athena/Items/ForagedItems/LowGravity/CBGA_LowGravity_UsingJump.CBGA_LowGravity_UsingJump_C");
};

class ACBGA_HealthSmall_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/ForagedItems/HealthSmall/CBGA_HealthSmall.CBGA_HealthSmall_C");
};

class ACBGA_ShieldsSmall_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/ForagedItems/ShieldsSmall/CBGA_ShieldsSmall.CBGA_ShieldsSmall_C");
};

class ACBGA_SpookyMist_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/ForagedItems/SpookyMist/CBGA_SpookyMist.CBGA_SpookyMist_C");
};

class AProp_QuestInteractable_TimeTrials_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/TimedChallenge/Prop_QuestInteractable_TimeTrials.Prop_QuestInteractable_TimeTrials_C");
};

class AProp_Athena_Doorbell_Interactable_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/Doorbell/Prop_Athena_Doorbell_Interactable.Prop_Athena_Doorbell_Interactable_C");

    DEFINE_PROPERTY(class UBoxComponent*, InsideHouseOverlap);

    DEFINE_PROPERTY(FName, ObjBackendName);
};

class AProp_FMP_GooseEgg_Interactable : public AActor
{
public:
    DEFINE_CLASS(L"/Game/fortmp/content_simple/athena/quests/Prop_FMP_GooseEgg_Interactable.Prop_FMP_GooseEgg_Interactable_C");

    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);
};

class AFortSprayDecalInstance : public ABuildingSMActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortSprayDecalInstance")

    void StartSprayFadeOutDueToNewPlacement()
    {
        STATIC_FINDFUNCTION(StartSprayFadeOutDueToNewPlacement)

        Call(Function_StartSprayFadeOutDueToNewPlacement);
    }
};

class ABP_SprayDecal_C : public AFortSprayDecalInstance
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/Cosmetics/Sprays/BP_SprayDecal.BP_SprayDecal_C")

    DEFINE_PROPERTY(AFortPlayerController*, SpawningPlayerController);
};