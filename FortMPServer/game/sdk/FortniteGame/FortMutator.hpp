#pragma once

struct FLocationLerpData
{
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_Start);
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_End);
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_Target);
    DEFINE_STRUCT_PROPERTY(float, TotalLerpTime);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.LocationLerpData");
};

struct FBarrierTeamState
{
   DEFINE_STRUCT_PROPERTY(uint8, TeamNum);         
   DEFINE_STRUCT_PROPERTY(uint8_t, FoodTeam);
   DEFINE_STRUCT_PROPERTY(class AAthenaBarrierFlag*, ObjectiveFlag);
   DEFINE_STRUCT_PROPERTY(class AAthenaBarrierObjective*, ObjectiveObject);   
   DEFINE_STRUCT_PROPERTY(bool, bRespawnEnabled);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.BarrierTeamState");
};

struct FHeistExitCraftData
{
    DEFINE_STRUCT_PROPERTY(class AFortAthenaExitCraftSpawner*, ExitCraftSpawner);
    DEFINE_STRUCT_PROPERTY(class AFortAthenaExitCraft*, SpawnedExitCraft);
    DEFINE_STRUCT_PROPERTY(uint8, ExitCraftState);
    DEFINE_STRUCT_PROPERTY(TArray<class AFortPlayerPawnAthena*>, DepartedPawns);
    DEFINE_STRUCT_PROPERTY(float, SpawnTime);
    DEFINE_STRUCT_PROPERTY(bool, bIsUsed);
    DEFINE_STRUCT_PROPERTY(bool, bHasDeparted);

    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.HeistExitCraftData");
};

struct FExitCraftInfo
{
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitCraftSpawnerZOffset);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitCraftZOffset);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitCraftTargetZOffset);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitZOffset);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitTime);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitCraftTimeToHoverLocation);
    DEFINE_STRUCT_PROPERTY(struct FScalableFloat, ExitCraftTimeToHoverRotation);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.ExitCraftInfo");
};

class AFortGameplayMutator : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameplayMutator");
    DEFINE_BIT_PROPERTY(bool, bMutatorActive);
};

class AFortAthenaMutator : public AFortGameplayMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator");
};

class AFortAthenaMutator_InventoryOverride : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_InventoryOverride")

    DEFINE_PROPERTY(uint8, DropAllItemsOverride);
    DEFINE_PROPERTY(uint8, TrapDropOverride);
    DEFINE_PROPERTY(uint8, WeaponDropOverride);
    DEFINE_PROPERTY(uint8, MaterialDropOverride);
    DEFINE_PROPERTY(uint8, GadgetDropOverride);
    DEFINE_PROPERTY(uint8, ConsumableDropOverride);
    DEFINE_PROPERTY(uint8, AmmoDropOverride);
};

class AFortAthenaMutator_Bots : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_Bots")

    class AFortPlayerPawnAthena* SpawnBot(class AFortPlayerPawnAthena* BotPawnClass, class AActor* InSpawnLocator, struct FVector& InSpawnLocation, struct FRotator& InSpawnRotation, bool bSnapToGround)
    {
        STATIC_FINDFUNCTION(SpawnBot)

        return Call<AFortPlayerPawnAthena*>(Function_SpawnBot, BotPawnClass, InSpawnLocator, InSpawnLocation, InSpawnRotation, bSnapToGround);
    }
};

class AFortAthenaExitCraftSpawner : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaExitCraftSpawner")

    DEFINE_PROPERTY(class UFortAthenaExitCraftInfo*, ExitCraftInfo);
    DEFINE_PROPERTY(FName, SpawnerSpecialActorID);
    
    void StartExitCraftSpawnTimer()
    {
        STATIC_FINDFUNCTION(StartExitCraftSpawnTimer)

        Call(Function_StartExitCraftSpawnTimer);
    }
};

class AAthenaBarrierObjective : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaBarrierObjective")

    DEFINE_PROPERTY(uint8_t, FoodTeam);
    DEFINE_PROPERTY(bool, bAllowDamage);
    DEFINE_PROPERTY(float, HeadRotationYaw);
    
    void OnRep_FoodTeam()
    {
        STATIC_FINDFUNCTION(OnRep_FoodTeam)
        Call(Function_OnRep_FoodTeam);
    }

    void OnRep_HeadRotationYaw()
    {
        STATIC_FINDFUNCTION(OnRep_HeadRotationYaw)
        Call(Function_OnRep_HeadRotationYaw);
    }

    void OnRep_ObjectiveDamageState()
    {
        STATIC_FINDFUNCTION(OnRep_ObjectiveDamageState)
        Call(Function_OnRep_ObjectiveDamageState);
    }
    
    void UpdateInGameHealth(float NewHealthPercent)
    {
        STATIC_FINDFUNCTION(UpdateInGameHealth)
        Call(Function_UpdateInGameHealth, NewHealthPercent);
    }

    void OnHeadRotationSet(float NewYaw)
    {
        STATIC_FINDFUNCTION(OnHeadRotationSet)
        Call(Function_OnHeadRotationSet, NewYaw);
    }
};

class AAthenaBarrierFlag : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaBarrierFlag")

    DEFINE_PROPERTY(uint8_t, FoodTeam);
    DEFINE_PROPERTY(uint8_t, CurrentState);

    void OnRep_FoodTeam()
    {
        STATIC_FINDFUNCTION(OnRep_FoodTeam)
        Call(Function_OnRep_FoodTeam);
    }

    AAthenaBarrierObjective* GetObjectiveActor()
    {
        STATIC_FINDFUNCTION(GetObjectiveActor)
        return Call<AAthenaBarrierObjective*>(Function_GetObjectiveActor);
    }
};

class AAthenaBigBaseWall : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaBigBaseWall")

    DEFINE_PROPERTY(uint8_t, BarrierState);
    DEFINE_PROPERTY(float, TimeUntilWallComesDown);

    void OnNewBarrierState(uint8_t NewState)
    {
        STATIC_FINDFUNCTION(OnNewBarrierState)
        Call(Function_OnNewBarrierState, NewState);
    }

    void WallStartingToComeDown(bool bIsOnBurgerSide)
    {
        STATIC_FINDFUNCTION(WallStartingToComeDown)
        Call(Function_WallStartingToComeDown, bIsOnBurgerSide);
    }
    
    void UpdateWallTime(float Digit_0, float Digit_1, float Digit_2, float Digit_3)
    {
        STATIC_FINDFUNCTION(UpdateWallTime)
        Call(Function_UpdateWallTime, Digit_0, Digit_1, Digit_2, Digit_3);
    }
    
    void ShowOrHideTimer(bool bNewVisibleState)
    {
        STATIC_FINDFUNCTION(ShowOrHideTimer)
        Call(Function_ShowOrHideTimer, bNewVisibleState);
    }

    void OnRep_BarrierState()
    {
        STATIC_FINDFUNCTION(OnRep_BarrierState)
        Call(Function_OnRep_BarrierState);
    }

    void OnRep_TimeUntilWallComesDown()
    {
        STATIC_FINDFUNCTION(OnRep_TimeUntilWallComesDown)
        Call(Function_OnRep_TimeUntilWallComesDown);
    }
};

class AFortAthenaMutator_Barrier : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_Barrier")

    DEFINE_PROPERTY(UObject*, BigBaseWallClass);
    DEFINE_PROPERTY(UObject*, ObjectiveFlag);
    
    DEFINE_PROPERTY(AAthenaBigBaseWall*, BigBaseWall);

    DEFINE_PROPERTY(FBarrierTeamState, Team_0_State);
    DEFINE_PROPERTY(FBarrierTeamState, Team_1_State);
    
    DEFINE_PROPERTY(FScalableFloat, TimeToBringDownWall);
    DEFINE_PROPERTY(FScalableFloat, ObjectiveDistanceFromWall);

    AAthenaBarrierFlag* SpawnBarrierObjective(uint8_t team)
    {
        // Spawn the barrier objective for both teams
        FActorSpawnParameters spawn_parameters = {};
	
        FVector objective_spawn_location = FVector((*GWorld)->AuthorityGameMode()->SafeZoneLocations()[1].X, (*GWorld)->AuthorityGameMode()->SafeZoneLocations()[1].Y, 5000);
        FRotator objective_spawn_rotation = FRotator();

        FVector right_barrier_vector = this->BigBaseWall()->GetActorRightVector();
        float objective_distance_from_barrier = ObjectiveDistanceFromWall().GetValueAtLevel(0, nullptr);

        // Make the tomato team objective spawn on the other side than the burger objective
        if (team == 1) objective_distance_from_barrier *=  -1;
        
        right_barrier_vector.X *= objective_distance_from_barrier;
        right_barrier_vector.Y *= objective_distance_from_barrier;
        right_barrier_vector.Z *= objective_distance_from_barrier;
 
        objective_spawn_location = UKismetMathLibrary::Add_VectorVector(right_barrier_vector, objective_spawn_location);

        // Match to the grid
        objective_spawn_location.X /= 512;
        objective_spawn_location.Y /= 512;
        objective_spawn_location.X = round(objective_spawn_location.X);
        objective_spawn_location.Y = round(objective_spawn_location.Y);
        objective_spawn_location.X *= 512;
        objective_spawn_location.Y *= 512;

        auto objective_flag = static_cast<AAthenaBarrierFlag*>(SpawnActor(*GWorld, this->ObjectiveFlag(), &objective_spawn_location, &objective_spawn_rotation, spawn_parameters));

        objective_flag->CurrentState() = 0;
        auto objective_object = objective_flag->GetObjectiveActor();

        if (objective_object == nullptr)
        {
            return nullptr;
        }
        
        if (team == 0)
        {
            Team_0_State().TeamNum() = 0;
            Team_0_State().FoodTeam() = 0;
            Team_0_State().ObjectiveFlag() = objective_flag;
            objective_flag->FoodTeam() = 0;
            objective_object->FoodTeam() = 0;
        }
        else
        {
            Team_1_State().TeamNum() = 1;
            Team_1_State().FoodTeam() = 1;
            Team_1_State().ObjectiveFlag() = objective_flag;
            objective_flag->FoodTeam() = 1;
            objective_object->FoodTeam() = 1;
        }
        objective_object->bAllowDamage() = true;
        objective_object->OnRep_ObjectiveDamageState();
        objective_flag->OnRep_FoodTeam();
        objective_object->OnRep_FoodTeam();
        objective_object->UpdateInGameHealth(100);
        
        return objective_flag;
    }
};

class AFortAthenaMutator_Heist : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_Heist")

    DEFINE_PROPERTY(class UFortAthenaExitCraftInfo*, ExitCraftInfo);
    DEFINE_PROPERTY(TArray<struct FHeistExitCraftData>, SpawnedExitCraftList);
    DEFINE_PROPERTY(FGameplayTag, ExitCraftTag);
    DEFINE_PROPERTY(int, NumSpawnedExitCrafts);
    DEFINE_PROPERTY(int, NumUnspawnedExitCrafts);

    void OnSpecialActorAdded(struct FSpecialActorRepData SpecialActorData)
    {
        STATIC_FINDFUNCTION(OnSpecialActorAdded)

        Call(Function_OnSpecialActorAdded, SpecialActorData);
        
    }

    void OnSpecialActorRemoved(struct FSpecialActorRepData SpecialActorData)
    {
        STATIC_FINDFUNCTION(OnSpecialActorRemoved)

        Call(Function_OnSpecialActorRemoved, SpecialActorData);
    }
    
    void OnExitCraftSpawned(class AFortAthenaExitCraft* ExitCraft, class AFortAthenaExitCraftSpawner* ExitCraftSpawner)
    {
        STATIC_FINDFUNCTION(OnExitCraftSpawned)

        Call(Function_OnExitCraftSpawned, ExitCraft, ExitCraftSpawner);
    }

    FHeistExitCraftData* FindCraftData(AFortAthenaExitCraftSpawner* Spawner)
    {
        auto spawned_exit_craft_list = TArrayManager<FHeistExitCraftData>(&SpawnedExitCraftList(), FHeistExitCraftData::GetStructSize());
        
        for (FHeistExitCraftData& CraftData : spawned_exit_craft_list)
        {
            if (CraftData.ExitCraftSpawner() == Spawner)
            {
                return &CraftData;
            }
        }
        return nullptr;
    }
};

class UFortAthenaExitCraftInfo : public UObject
{
public:
    // Fortnite Typo, do not correct.
    DEFINE_PROPERTY(UObject*, ExitCaftClass);
    DEFINE_PROPERTY(FExitCraftInfo, ExitCraftInfo);
    DEFINE_PROPERTY(struct FSlateBrush, CraftMinimapIconBrush);
    DEFINE_PROPERTY(FVector2D, CraftMinimapIconScale);
    DEFINE_PROPERTY(struct FSlateBrush, CraftCompassIconBrush);
    DEFINE_PROPERTY(FVector2D, CraftCompassIconScale);
};

class AFortAthenaExitCraft : public AFortAthenaVehicle
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaExitCraft")

    DEFINE_PROPERTY(uint8, CurrentState);
    DEFINE_PROPERTY(class UFortAthenaExitCraftInfo*, ExitCraftInfo);
    DEFINE_PROPERTY(FLocationLerpData, LocationLerpData);
    DEFINE_PROPERTY(FName, CraftSpecialActorID);
    
    void OnMoveEnd()
    {
        STATIC_FINDFUNCTION(OnMoveEnd)

        Call(Function_OnMoveEnd);
    }

    void OnNewState(uint8 NewState)
    {
        STATIC_FINDFUNCTION(OnNewState)

        Call(Function_OnNewState, NewState);
    }

    void OnRep_CurrentState()
    {
        STATIC_FINDFUNCTION(OnRep_CurrentState)

        Call(Function_OnRep_CurrentState);
    }

    void OnRep_LocationLerpData()
    {
        STATIC_FINDFUNCTION(OnRep_LocationLerpData)

        Call(Function_OnRep_LocationLerpData);
    }

    void OnRep_RotationLerpData()
    {
        STATIC_FINDFUNCTION(OnRep_RotationLerpData)

        Call(Function_OnRep_RotationLerpData);
    }
};