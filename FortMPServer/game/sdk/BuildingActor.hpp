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
};

class ABuildingSMActor : public ABuildingActor
{
public:
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

    void OnRep_bAlreadySearched()
    {
        Call(L"OnRep_bAlreadySearched");
    }
};