#pragma once

class UGameplayEffect : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/GameplayAbilities.GameplayEffect")
};

struct FActiveGameplayEffectHandle
{
    char Padding[0x8];
};

struct FGameplayEffectContext
{
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<UObject>, SourceObject);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<UObject>, Instigator);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<UObject>, EffectCauser);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayEffectContext")
};

struct FGameplayEffectContextHandle
{
    TSharedPtr<FGameplayEffectContext> Data;
};

struct FGameplayEffectSpec
{
    DEFINE_STRUCT_PROPERTY(FGameplayEffectContextHandle, EffectContext);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayEffectSpec");
};

struct FGameplayEffectSpecHandle
{
    TSharedPtr<FGameplayEffectSpec> Data;

    void AddSourceObject(UObject* SourceObject)
    {
        if (FGameplayEffectSpec* Spec = Data.Get())
        {
            if (FGameplayEffectContext* Context = Spec->EffectContext().Data.Get())
            {
                Context->SourceObject() = SourceObject;
            }
        }
    }
};

struct FGameplayEffectApplicationInfo
{
    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr<UObject>, GameplayEffect);
    DEFINE_STRUCT_PROPERTY(float, Level);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.GameplayEffectApplicationInfo")
};

struct FFortDeliveryInfoRequirementsFilter
{
    DEFINE_STRUCT_BIT_PROPERTY(bool, bApplyToPlayerPawns);
    DEFINE_STRUCT_BIT_PROPERTY(bool, bApplyToAIPawns);
    DEFINE_STRUCT_BIT_PROPERTY(bool, bApplyToBuildingActors);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortDeliveryInfoRequirementsFilter")
};

struct FFortGameplayEffectDeliveryInfo
{
    DEFINE_STRUCT_PROPERTY(TArray<FGameplayEffectApplicationInfo>, GameplayEffects);
    DEFINE_STRUCT_PROPERTY(FFortDeliveryInfoRequirementsFilter,  DeliveryRequirements);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortGameplayEffectDeliveryInfo")
};

struct FFortAbilitySetDeliveryInfo
{
    DEFINE_STRUCT_PROPERTY(TArray<TSoftObjectPtr<UObject>>, AbilitySets);
    DEFINE_STRUCT_PROPERTY(FFortDeliveryInfoRequirementsFilter,  DeliveryRequirements);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortAbilitySetDeliveryInfo")
};

struct FGameplayAbilityTargetDataHandle
{
    uint8 UniqueId;

    char pad[0x19];

    TArray<TSharedPtr<struct FFortGameplayAbilityTargetData_SingleTargetHit>>& GetData()
    {
        return *reinterpret_cast<TArray<TSharedPtr<FFortGameplayAbilityTargetData_SingleTargetHit>>*>(reinterpret_cast<uintptr_t>(this) + (GetEngineVersionEasy() > 422 ? 0x18 : 0x10));
    }
    
    TSharedPtr<struct FFortGameplayAbilityTargetData_SingleTargetHit>& GetSingleData()
    {
        return *reinterpret_cast<TSharedPtr<FFortGameplayAbilityTargetData_SingleTargetHit>*>(reinterpret_cast<uintptr_t>(this) + (GetEngineVersionEasy() > 422 ? 0x8 : 0x0));
    }
    
    bool IsValid(int32 Index)
    {
        return (GetData().Data && Index < GetData().Num() && GetData()[Index].IsValid());
    }

    FFortGameplayAbilityTargetData_SingleTargetHit* Get(int32 Index)
    {
        return IsValid(Index) ? GetData()[Index].Get() : nullptr;
    }

    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayAbilityTargetDataHandle")
};

struct FFortAbilityCost
{
    DEFINE_STRUCT_PROPERTY(uint8_t, CostSource);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortAbilityCost");
};

class UGameplayAbility : public UObject
{
public:
    DEFINE_PROPERTY(uint8_t, InstancingPolicy);
    
    void K2_CancelAbility()
    {
        STATIC_FINDFUNCTION(K2_CancelAbility)
        
        Call(Function_K2_CancelAbility);
    }

    void K2_RemoveGameplayCue(FGameplayTag CueTag)
    {
        STATIC_FINDFUNCTION(K2_RemoveGameplayCue)

        Call(Function_K2_RemoveGameplayCue, CueTag);
    }

    AActor* GetAvatarActorFromActorInfo()
    {
        STATIC_FINDFUNCTION(GetAvatarActorFromActorInfo)

        return Call<AActor*>(Function_GetAvatarActorFromActorInfo);
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

struct FGameplayCueParameters
{
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayCueParameters")
    DEFINE_STRUCT_PROPERTY(float, NormalizedMagnitude);
    DEFINE_STRUCT_PROPERTY(float, RawMagnitude);
    DEFINE_STRUCT_PROPERTY(FGameplayTag, MatchedTagName);
    DEFINE_STRUCT_PROPERTY(FGameplayTag, OriginalTag);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, AggregatedSourceTags);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, AggregatedTargetTags);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<class AActor>, Instigator);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<class AActor>, EffectCauser);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<class AActor>, SourceObject);
};

class UFortGameplayAbilityAthena_PeriodicItemGrant : public UFortGameplayAbility
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant");
};