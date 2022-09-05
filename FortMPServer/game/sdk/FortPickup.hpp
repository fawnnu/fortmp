#pragma once

struct FFortPickupLocationData
{
    DEFINE_STRUCT_PROPERTY(uint8_t, TossState);
    DEFINE_STRUCT_PROPERTY(float, FlyTime);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, ItemOwner);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, PickupTarget);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPickupLocationData");
};

class AFortPickup : public AActor
{
public:
    DEFINE_PROPERTY(bool, bServerStoppedSimulation);
    DEFINE_PROPERTY(bool, bRandomRotation);
    DEFINE_PROPERTY(bool, bPickedUp);

    DEFINE_PROPERTY(TWeakObjectPtr<AActor>, PickupEffectBlueprint);
    DEFINE_PROPERTY(FFortItemEntry, PrimaryPickupItemEntry);
    DEFINE_PROPERTY(FFortPickupLocationData, PickupLocationData);

    DEFINE_PROPERTY(class UProjectileMovementComponent*, MovementComponent);
    DEFINE_PROPERTY(class UPrimitiveComponent*, TouchCapsule);

    DEFINE_PROPERTY(bool, bTossedFromContainer);
    DEFINE_PROPERTY(bool, bCombinePickupsWhenTossCompletes);

    void TossPickup(FVector FinalLocation, AFortPlayerPawn* ItemOwner, int OverrideMaxStackCount, bool bToss)
    {
        static bool b_has_toss_argument = FindPropertyByName(FindObject(L"/Script/FortniteGame.FortPickup.TossPickup"), L"bToss") != nullptr;
        
        b_has_toss_argument ?
            Call(L"TossPickup", FinalLocation, ItemOwner, OverrideMaxStackCount, bToss) :
            Call(L"TossPickup", FinalLocation, ItemOwner, OverrideMaxStackCount);
    }

    void OnRep_PickupLocationData()
    {
        Call(L"OnRep_PickupLocationData");
    }
};