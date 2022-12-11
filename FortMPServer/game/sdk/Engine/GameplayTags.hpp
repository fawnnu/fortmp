#pragma once

struct FGameplayTag
{
    FName TagName;
};

struct FGameplayTagContainer
{
    TArray<FGameplayTag> GameplayTags;
    TArray<FGameplayTag> ParentTags;
};

struct FGameplayEventData
{
    static FGameplayEventData* Create()
    {
        FGameplayEventData* ReturnValue = static_cast<FGameplayEventData*>(alloca(GetStructSize()));

        ZeroMemory(ReturnValue, GetStructSize());

        return ReturnValue;
    }
    
    DEFINE_STRUCT_PROPERTY(FGameplayTag, EventTag);
    DEFINE_STRUCT_PROPERTY(AActor*, Instigator);
    DEFINE_STRUCT_PROPERTY(AActor*, Target);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayEventData");
};