#pragma once

class AFortPickup : public UObject
{
public:
    DEFINE_PROPERTY(bool, bServerStoppedSimulation);
    DEFINE_PROPERTY(bool, bRandomRotation);
    DEFINE_PROPERTY(bool, bPickedUp);
    
    DEFINE_PROPERTY(FFortItemEntry, PrimaryPickupItemEntry);

    void TossPickup(FVector FinalLocation, AFortPlayerPawn* ItemOwner, int OverrideMaxStackCount, bool bToss)
    {
        Call(L"TossPickup", FinalLocation, ItemOwner, OverrideMaxStackCount, bToss);
    }
};