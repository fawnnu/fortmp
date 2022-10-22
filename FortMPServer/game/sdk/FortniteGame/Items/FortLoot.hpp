#pragma once

struct FFortLootTierData
{
    DEFINE_STRUCT_PROPERTY(FName, TierGroup);
    DEFINE_STRUCT_PROPERTY(FName, LootPackage);
    
    DEFINE_STRUCT_PROPERTY(float, Weight);
    DEFINE_STRUCT_PROPERTY(float, NumLootPackageDrops);

    DEFINE_STRUCT_PROPERTY(int, LootTier);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortLootTierData");
};

struct FFortLootPackageData
{
    DEFINE_STRUCT_PROPERTY(FName, LootPackageID);
    DEFINE_STRUCT_PROPERTY(FString, LootPackageCall);

    DEFINE_STRUCT_PROPERTY(int, Count);
    DEFINE_STRUCT_PROPERTY(float, Weight);

    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr<UFortItemDefinition>, ItemDefinition);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortLootPackageData");
};