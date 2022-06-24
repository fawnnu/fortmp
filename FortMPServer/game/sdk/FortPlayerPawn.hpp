#pragma once

class AFortPlayerPawn : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, Owner);
};

class AFortAIPawn : public UObject
{
public:
    void AddFortAbilitySet(UObject* FortAbilitySet)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/FortniteGame.FortAIPawn.AddFortAbilitySet");
        
        // TODO: use Call
        ProcessEvent(this, function, &FortAbilitySet);
    }
};
