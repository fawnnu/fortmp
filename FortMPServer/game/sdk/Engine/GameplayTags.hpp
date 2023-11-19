#pragma once

struct FGameplayTag
{
    FGameplayTag()
    {
        TagName = FName();
    }

    FGameplayTag(FName InTagName)
    {
        TagName = InTagName;
    }

    bool operator==(FGameplayTag cmp)
    {
        return cmp.TagName == TagName;
    }

    bool operator==(const FGameplayTag cmp) const
    {
        return cmp.TagName == TagName;
    }
    
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
        FGameplayEventData* ReturnValue = static_cast<FGameplayEventData*>(malloc(GetStructSize()));

        ZeroMemory(ReturnValue, GetStructSize());

        return ReturnValue;
    }
    
    DEFINE_STRUCT_PROPERTY(FGameplayTag, EventTag);
    DEFINE_STRUCT_PROPERTY(AActor*, Instigator);
    DEFINE_STRUCT_PROPERTY(AActor*, Target);
    DEFINE_STRUCT_PROPERTY(AActor*, OptionalObject);
    DEFINE_STRUCT_PROPERTY(AActor*, OptionalObject2);
    DEFINE_STRUCT_PROPERTY(float, EventMagnitude);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayEventData");
};