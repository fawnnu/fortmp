#pragma once

struct FGameplayAbilityTargetDataHandle
{
    uint8 UniqueId;

    char pad[0x19];

    TArray<TSharedPtr<struct FFortGameplayAbilityTargetData_SingleTargetHit>>& GetData()
    {
        return *reinterpret_cast<TArray<TSharedPtr<FFortGameplayAbilityTargetData_SingleTargetHit>>*>(reinterpret_cast<uintptr_t>(this) + 0x18);
    }
    
    bool IsValid(int32 Index)
    {
        return (GetData().Data && Index < GetData().Num() && GetData()[Index].IsValid());
    }

    FFortGameplayAbilityTargetData_SingleTargetHit* Get(int32 Index)
    {
        return IsValid(Index) ? GetData()[Index].Get() : nullptr;
    }
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
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameplayAbility_RangedWeapon");
    
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
};