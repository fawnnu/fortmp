#include "include.hpp"

FGameplayAbilityTargetDataHandle* UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(AActor* Actor)
{
    static UObject* Default__AbilitySystemBlueprintLibrary = FindObject(L"/Script/GameplayAbilities.Default__AbilitySystemBlueprintLibrary");

    STATIC_FINDFUNCTIONOBJECT(L"/Script/GameplayAbilities.AbilitySystemBlueprintLibrary.AbilityTargetDataFromActor");

    // GET PARAMS SIZE
    int16_t parms_size = 0;
            
    for (UObject* Property = *reinterpret_cast<UObject**>(__int64(FunctionObject) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(__int64(Property) + CoreOffsets::Next))
    {
        int offset = *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::Offset_Internal);
            
        parms_size = offset + // Offset_Internal
            *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::ElementSize); // ElementSize
    }

    // SET PARAMS
    struct FunctionParams
    {
        AActor* Actor;
        FGameplayAbilityTargetDataHandle ReturnValue;
    };

    FunctionParams* params = static_cast<FunctionParams*>(alloca(parms_size));
    params->Actor = Actor;

    ProcessEvent(Default__AbilitySystemBlueprintLibrary, FunctionObject, params);

    FGameplayAbilityTargetDataHandle* ReturnValue = static_cast<FGameplayAbilityTargetDataHandle*>(malloc(FGameplayAbilityTargetDataHandle::GetStructSize()));
    memcpy(ReturnValue, &params->ReturnValue, FGameplayAbilityTargetDataHandle::GetStructSize());

    return ReturnValue;
}

FGameplayEffectSpecHandle* UAbilitySystemBlueprintLibrary::MakeSpecHandle(UObject* InGameplayEffect, AActor* InInstigator, AActor* InEffectCauser, float InLevel)
{
    static UObject* Default__AbilitySystemBlueprintLibrary = FindObject(L"/Script/GameplayAbilities.Default__AbilitySystemBlueprintLibrary");

    STATIC_FINDFUNCTIONOBJECT(L"/Script/GameplayAbilities.AbilitySystemBlueprintLibrary.MakeSpecHandle");

    struct 
    {
        UObject* InGameplayEffect;              
        AActor* InInstigator;
        AActor* InEffectCauser;
        float InLevel;
        FGameplayEffectSpecHandle ReturnValue;
    } FunctionParams;
    
    FunctionParams.InGameplayEffect = InGameplayEffect;
    FunctionParams.InInstigator = InInstigator;
    FunctionParams.InEffectCauser = InEffectCauser;
    FunctionParams.InLevel = InLevel;
    
    ProcessEvent(Default__AbilitySystemBlueprintLibrary, FunctionObject, &FunctionParams);

    FGameplayEffectSpecHandle* ReturnValue = static_cast<FGameplayEffectSpecHandle*>(malloc(sizeof FGameplayEffectSpecHandle));
    memcpy(ReturnValue, &FunctionParams.ReturnValue, sizeof FGameplayEffectSpecHandle);
    
    return ReturnValue;
}
