#pragma once

struct FGameplayAbilityTargetDataHandle
{
    char UnknownData_00[0x20];
};

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
        STATIC_FINDFUNCTION(K2_CancelAbility)
        
        Call(Function_K2_CancelAbility);
    }
};

class UFortGameplayAbility : public UGameplayAbility
{
public:
    DEFINE_PROPERTY(TArray<FFortAbilityCost>, AbilityCosts);

    AFortPawn* GetActivatingPawn()
    {
        STATIC_FINDFUNCTION(GetActivatingPawn)
        
        return Call<AFortPawn*>(Function_GetActivatingPawn);
    }

    AFortWeapon* GetCurrentSourceWeapon()
    {
        STATIC_FINDFUNCTION(GetCurrentSourceWeapon)
        
        return Call<AFortWeapon*>(Function_GetCurrentSourceWeapon);
    }
};

class UFortGameplayAbility_RangedWeapon : public UFortGameplayAbility
{
public:
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
};