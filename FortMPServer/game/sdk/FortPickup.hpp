#pragma once

struct FFortPickupLocationData
{
    DEFINE_STRUCT_PROPERTY(uint8_t, TossState);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, PickupTarget);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPickupLocationData");
};

class AFortPickup : public AActor
{
public:
    DEFINE_PROPERTY(bool, bServerStoppedSimulation);
    DEFINE_PROPERTY(bool, bRandomRotation);
    DEFINE_PROPERTY(bool, bPickedUp);
    
    DEFINE_PROPERTY(FFortItemEntry, PrimaryPickupItemEntry);
    DEFINE_PROPERTY(FFortPickupLocationData, PickupLocationData);

    void TossPickup(FVector FinalLocation, AFortPlayerPawn* ItemOwner, int OverrideMaxStackCount, bool bToss)
    {
        Call(L"TossPickup", FinalLocation, ItemOwner, OverrideMaxStackCount, bToss);
    }
};