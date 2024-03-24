#pragma once

class UFortGameplayModifierItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(TArray<struct FFortGameplayEffectDeliveryInfo>, PersistentGameplayEffects);
    DEFINE_PROPERTY(TArray<struct FFortAbilitySetDeliveryInfo>, PersistentAbilitySets);
    
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameplayModifierItemDefinition")
};

class UFortPlaylist : public UObject
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootTierData);
    DEFINE_PROPERTY(TSoftObjectPtr<class UDataTable>, LootPackages);
    DEFINE_PROPERTY(TSoftObjectPtr<class UCurveTable>, GameData);
    DEFINE_PROPERTY(TSoftObjectPtr<class UCurveTable>, ResourceRates);
    DEFINE_PROPERTY(uint8_t, DefaultFirstTeam);
    DEFINE_PROPERTY(uint8_t, DefaultLastTeam);
    DEFINE_PROPERTY(int, MaxTeamSize);
    DEFINE_PROPERTY(int, MaxSquadSize);
    DEFINE_PROPERTY(int, MaxTeamCount);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortGameplayModifierItemDefinition>>, ModifierList);
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlaylistAthena");
    DEFINE_PROPERTY(struct FScalableFloat, RespawnHeight);

    DEFINE_PROPERTY(uint8_t, RespawnType);
    DEFINE_PROPERTY(uint8_t, AirCraftBehavior);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<class UObject>>, AdditionalLevels);
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<class UObject>>, AdditionalLevelsServerOnly);
    DEFINE_BIT_PROPERTY(bool, bEnableCreativeMode);
    DEFINE_PROPERTY(bool, bIsLargeTeamGame);
    DEFINE_PROPERTY(int, PlaylistId);
};