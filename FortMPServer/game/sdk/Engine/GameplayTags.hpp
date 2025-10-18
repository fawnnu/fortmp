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

    bool operator==(FName cmp)
    {
        return cmp == TagName;
    }

    bool operator==(const FName cmp) const
    {
        return cmp == TagName;
    }

    FORCEINLINE bool operator<(FGameplayTag cmp) const
    {
        return TagName < cmp.TagName;
    }
    
    FORCEINLINE operator const char*() 
    {
        return TagName.sToString().c_str();
    }

    FORCEINLINE operator const wchar_t*()
    {
        return TagName.ToString().c_str();
    }
    
    FName TagName;
};

struct FGameplayTagContainer
{
    FGameplayTagContainer() = default;
    
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
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, InstigatorTags);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, TargetTags);
    DEFINE_STRUCT_PROPERTY(float, EventMagnitude);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayEventData");
};