#pragma once
#include "../FortGameMode.hpp"
#include "../../Engine/World.hpp"
#include "../../Engine/GameplayAbilities/AbilitySystemComponent.hpp"
#include "../../Engine/Kismet/GameplayStatics.hpp"
#include "../../Engine/Kismet/KismetLibrary.hpp"

struct FFortSupplyDropSubPhaseData
{
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropMinCount);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropMaxCount);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropCap);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortSupplyDropSubPhaseData");
};

struct FFortSupplyDropGamePhaseData
{
    DEFINE_STRUCT_PROPERTY(uint8_t, GamePhase);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropTimeInterval);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropTimeDeviation);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropMinPlacementHeight);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, SupplyDropMaxPlacementHeight);
    DEFINE_STRUCT_PROPERTY(TArray<FFortSupplyDropSubPhaseData>, SubPhaseData);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortSupplyDropGamePhaseData");
};

struct FVehicleClassDetails
{
    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr<AFortAthenaVehicle>, VehicleClass);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, VehicleMinSpawnPercent);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, VehicleMaxSpawnPercent);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.VehicleClassDetails");
};

class UFortSupplyDropInfo : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, SupplyDropClass);
    DEFINE_PROPERTY(TArray<FFortSupplyDropGamePhaseData>, GamePhaseData);
};

struct FFortSafeZoneDefinition
{
    DEFINE_STRUCT_PROPERTY(FScalableFloat, WaitTime);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, ShrinkTime);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortSafeZoneDefinition");
};

class AFortAthenaMapInfo : public AActor
{
public:
    DEFINE_PROPERTY(TArray<UFortSupplyDropInfo*>, SupplyDropInfoList);

    DEFINE_PROPERTY(TArray<struct FVehicleClassDetails>, VehicleClassDetails);
    
    DEFINE_PROPERTY(UObject*, LlamaClass);
    DEFINE_PROPERTY(FScalableFloat, LlamaQuantityMin);
    DEFINE_PROPERTY(FScalableFloat, LlamaQuantityMax);

    DEFINE_PROPERTY(UObject*, TreasureChestClass);
    DEFINE_PROPERTY(FScalableFloat, TreasureChestMinSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, TreasureChestMaxSpawnPercent);
    
    DEFINE_PROPERTY(UObject*, AmmoBoxClass);
    DEFINE_PROPERTY(FScalableFloat, AmmoBoxMinSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, AmmoBoxMaxSpawnPercent);

    DEFINE_PROPERTY(FScalableFloat, VendingMachineRarityCount);
    DEFINE_PROPERTY(FScalableFloat, SafeZoneStartDelay);
};