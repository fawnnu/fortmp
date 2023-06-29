#pragma once
#include "Pawns/FortPlayerPawn.hpp"

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

class AFortAthenaMutator_Heist : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_Heist")

    DEFINE_PROPERTY(class UFortAthenaExitCraftInfo*, ExitCraftInfo);

    void OnExitCraftSpawned(class AFortAthenaExitCraft* ExitCraft, class AFortAthenaExitCraftSpawner* ExitCraftSpawner)
    {
        STATIC_FINDFUNCTION(OnExitCraftSpawned)

        Call(Function_OnExitCraftSpawned, ExitCraft, ExitCraftSpawner);
    }
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

class UFortAthenaExitCraftInfo : public UObject
{
public:
    // Fortnite Typo, do not correct.
    DEFINE_PROPERTY(UObject*, ExitCaftClass);
    DEFINE_PROPERTY(FExitCraftInfo, ExitCraftInfo);
};

class AFortAthenaExitCraftSpawner : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaExitCraftSpawner")

    DEFINE_PROPERTY(UFortAthenaExitCraftInfo*, ExitCraftInfo);
};

struct FLocationLerpData
{
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_Start);
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_End);
    DEFINE_STRUCT_PROPERTY(FVector, PositionLerp_Target);
    DEFINE_STRUCT_PROPERTY(float, TotalLerpTime);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.LocationLerpData");
};


class AFortAthenaExitCraft : public AFortAthenaVehicle
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaExitCraft")

    DEFINE_PROPERTY(uint8, CurrentState);
    DEFINE_PROPERTY(class UFortAthenaExitCraftInfo*, ExitCraftInfo);
    DEFINE_PROPERTY(FLocationLerpData, LocationLerpData);
    
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