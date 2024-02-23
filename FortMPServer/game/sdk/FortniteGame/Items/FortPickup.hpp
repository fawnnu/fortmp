#pragma once

struct FFortPickupLocationData
{
    DEFINE_STRUCT_PROPERTY(AFortPickup*, CombineTarget);
    DEFINE_STRUCT_PROPERTY(uint8_t, TossState);
    DEFINE_STRUCT_PROPERTY(FVector, FinalTossRestLocation);
    DEFINE_STRUCT_PROPERTY(float, FlyTime);
    DEFINE_STRUCT_PROPERTY(FVector, LootFinalPosition);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, ItemOwner);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, PickupTarget);
    DEFINE_STRUCT_PROPERTY(FGuid, PickupGuid);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPickupLocationData");
};

class AFortPickup : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPickup")
    
    DEFINE_PROPERTY(bool, bServerStoppedSimulation);
    DEFINE_PROPERTY(bool, bRandomRotation);
    DEFINE_PROPERTY(bool, bPickedUp);

    DEFINE_PROPERTY(UFortWorldItem*, PrimaryPickupDummyItem);
    DEFINE_PROPERTY(TWeakObjectPtr<AActor>, PickupEffectBlueprint);
    DEFINE_PROPERTY(FFortItemEntry, PrimaryPickupItemEntry);
    DEFINE_PROPERTY(TArray<struct FFortItemEntry>, MultiItemPickupEntries);
    DEFINE_PROPERTY(FFortPickupLocationData, PickupLocationData);

    DEFINE_PROPERTY(class UProjectileMovementComponent*, MovementComponent);
    DEFINE_PROPERTY(class UPrimitiveComponent*, TouchCapsule);

    DEFINE_PROPERTY(bool, bTossedFromContainer);
    DEFINE_PROPERTY(bool, bCombinePickupsWhenTossCompletes);
    DEFINE_PROPERTY(AFortPawn*, PawnWhoDroppedPickup);

    void TossPickup(FVector FinalLocation, AFortPlayerPawn* ItemOwner, int OverrideMaxStackCount, bool bToss)
    {
        static bool b_has_toss_argument = FindPropertyByName(FindObject(L"/Script/FortniteGame.FortPickup.TossPickup"), L"bToss") != nullptr;

        STATIC_FINDFUNCTION(TossPickup)
        
        b_has_toss_argument ?
            Call(Function_TossPickup, FinalLocation, ItemOwner, OverrideMaxStackCount, bToss) :
            Call(Function_TossPickup, FinalLocation, ItemOwner, OverrideMaxStackCount);
    }

    void OnRep_PickupLocationData()
    {
        STATIC_FINDFUNCTION(OnRep_PickupLocationData)
        
        Call(Function_OnRep_PickupLocationData);
    }

    void OnRep_PrimaryPickupItemEntry()
    {
        STATIC_FINDFUNCTION(OnRep_PrimaryPickupItemEntry)
        
        Call(Function_OnRep_PrimaryPickupItemEntry);
    }
};

class AFortGameModePickup : public AFortPickup
{
    
};

class AFortPickupAthena : public AFortPickup
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPickupAthena")
};