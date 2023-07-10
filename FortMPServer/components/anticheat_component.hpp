#pragma once

struct FRangedAbilityData
{
    FRangedAbilityData() = default;

    float LastShotTime;
    int TimesLogged;
    float TargetDataTime[8];
    int TargetDataTimeIndex;
    int TargetDataTimeCount;
};

class anticheat_component
{
public:
    anticheat_component();

    static void Initialize();
    static bool ValidateTargetData(UFortGameplayAbility_RangedWeapon* Ability);
    static bool ValidateTargetDataFrequency(FRangedAbilityData& AbilityData, UFortGameplayAbility_RangedWeapon* Ability, UWorld* World, float FiringRatePerSecond, float OptimalTimeDelta, float CustomTimeDilation);
    static float GetFiringRate(AFortWeapon* FortWeapon);
    static float GetBurstFiringRate(AFortWeapon* FortWeapon);

    inline static std::map<AFortPlayerController*, int> IllegalSniperActionsCount;
    inline static std::map<AFortWeapon*, int> IllegalWeaponActionsCount;
    inline static std::map<AFortWeapon*, float> LastFireTime;

    inline static std::vector<FRangedAbilityData> RangedAbilityDataList;
};

namespace anticheat_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerNotifyPawnHit, (AFortProjectileAthena* Self, FFrame& StackFrame));
}

namespace anticheat_component_offsets
{
    inline int GetBurstFiringRateOffset;
    inline int GetFiringRateOffset;
}