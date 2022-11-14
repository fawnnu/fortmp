#pragma once

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

class UFortSupplyDropInfo : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, SupplyDropClass);
    DEFINE_PROPERTY(TArray<FFortSupplyDropGamePhaseData>, GamePhaseData);
};

class AFortAthenaMapInfo : public AActor
{
public:
    DEFINE_PROPERTY(TArray<UFortSupplyDropInfo*>, SupplyDropInfoList);

    DEFINE_PROPERTY(UObject*, LlamaClass);
    DEFINE_PROPERTY(FScalableFloat, LlamaQuantityMin);
    DEFINE_PROPERTY(FScalableFloat, LlamaQuantityMax);

    DEFINE_PROPERTY(UObject*, TreasureChestClass);
    DEFINE_PROPERTY(FScalableFloat, TreasureChestMinSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, TreasureChestMaxSpawnPercent);
    
    DEFINE_PROPERTY(UObject*, AmmoBoxClass);
    DEFINE_PROPERTY(FScalableFloat, AmmoBoxMinSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, AmmoBoxMaxSpawnPercent);

    DEFINE_PROPERTY(FScalableFloat, SafeZoneStartDelay);
};