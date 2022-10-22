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