#pragma once

struct FFortAbilityCost
{
    DEFINE_STRUCT_PROPERTY(uint8_t, CostSource);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortAbilityCost");
};

class UGameplayAbility : public UObject
{
public:
    void K2_CancelAbility()
    {
        Call(L"K2_CancelAbility");
    }
};

class UFortGameplayAbility : public UGameplayAbility
{
public:
    DEFINE_PROPERTY(TArray<FFortAbilityCost>, AbilityCosts);

    AFortPawn* GetActivatingPawn()
    {
        return Call<AFortPawn*>(L"GetActivatingPawn");
    }

    AFortWeapon* GetCurrentSourceWeapon()
    {
        return Call<AFortWeapon*>(L"GetCurrentSourceWeapon");
    }
};