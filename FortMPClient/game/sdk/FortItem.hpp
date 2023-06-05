#pragma once

struct FAthenaRewardItemReference
{
    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr, ItemDefinition);
    DEFINE_STRUCT_PROPERTY(FString, TemplateId);
    DEFINE_STRUCT_PROPERTY(int, Quantity);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaRewardItemReference")
};

struct FAthenaRewardScheduleLevel
{
    DEFINE_STRUCT_PROPERTY(TArray<FAthenaRewardItemReference>, Rewards)
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaRewardScheduleLevel")
};

struct FAthenaRewardSchedule
{
    DEFINE_STRUCT_PROPERTY(TArray<FAthenaRewardScheduleLevel>, Levels)
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaRewardSchedule");
};

class UFortItemDefinition : public UObject
{
public:
    
};

class UFortAccountItemDefinition : public UFortItemDefinition
{
public:
    
};

class UAthenaSeasonItemDefinition : public UFortAccountItemDefinition
{
public:
    DEFINE_PROPERTY(FAthenaRewardSchedule, BookXpScheduleFree);
};