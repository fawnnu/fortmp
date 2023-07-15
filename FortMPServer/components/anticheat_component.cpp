#include "include.hpp"
#include "anticheat_component.hpp"

#include "looting_component.hpp"
#include "player_component.hpp"

anticheat_component::anticheat_component()
{
    Initialize();
    
    if (uintptr_t GetFiringRateAddress = memory_manager::get_function_start(memory_manager::find_pattern("49 8B DE 48 85 ? 74 ? 48 8D 93 ? ? ? ? 48 85 DB 75 03 49 8B D6 48 8B ? 48 8B ? FF 90")))
    {
        for (int i = 0; i < INT_MAX; i++)
        {
            static AFortWeaponRanged* FortWeaponRanged__DefaultObject = CreateDefaultObject<AFortWeaponRanged>();
        
            if (FortWeaponRanged__DefaultObject->VTable[i] == reinterpret_cast<PVOID>(GetFiringRateAddress))
            {
                anticheat_component_offsets::GetFiringRateOffset = i;
                anticheat_component_offsets::GetBurstFiringRateOffset = i + 1;

                break;
            }
        }
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(anticheat_component_hooks::ServerNotifyPawnHit, TEXT("/Script/FortniteGame.FortProjectileAthena.ServerNotifyPawnHit"), anticheat_component_hooks::ServerNotifyPawnHit_Hook)

    DetourTransactionCommit();
}

void anticheat_component::Initialize()
{
    RangedAbilityDataList.clear();
    RangedAbilityDataList.resize(100);
}

bool anticheat_component::ValidateTargetData(UFortGameplayAbility_RangedWeapon* Ability)
{
    if (Ability)
    {
        if (AFortWeapon* CurrentWeapon = Ability->CurrentWeapon())
        {
            // Get owning Controller & Pawn
            AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(CurrentWeapon->GetOwner());
            AFortPlayerController* Controller = Pawn->Controller();
            const int WorldPlayerID = Controller->PlayerState()->WorldPlayerId();
            if (WorldPlayerID >= RangedAbilityDataList.size())
            {
                RangedAbilityDataList.resize(WorldPlayerID + 1);
            }

            FRangedAbilityData& AbilityData = RangedAbilityDataList[WorldPlayerID];
            AbilityData.Update(Ability);
            
            if (FFortBaseWeaponStats* WeaponStats = looting_component::get_weapon_stats(CurrentWeapon->WeaponData()))
            {
                if (WeaponStats->CartridgePerFire() <= 1)
                {
                    float FiringRate = GetFiringRate(CurrentWeapon);

                    if (FiringRate > 0.0)
                    {
                        float FiringRatePerSecond = 1.0 / FiringRate;
                        if (!ValidateTargetDataFrequency(AbilityData, Ability, *GWorld, FiringRatePerSecond, std::fmaxf(std::fminf(FiringRatePerSecond - 0.1, FiringRatePerSecond * 0.5), 0.0), CurrentWeapon->CustomTimeDilation()))
                        {
                            if ((*GWorld)->TimeSeconds() - (*GWorld)->AuthorityGameMode()->LastHitch() > (*GWorld)->AuthorityGameMode()->AbilityRefireHitchCooldown())
                            {
                                // Log only first 10 times
                                if (AbilityData.TimesLogged < 10)
                                {
                                    spdlog::error(L"Player '{}' has failed ValidateTargetDataFrequency checks. Possible fire rate cheats.", Controller->MyPlayerInfo()->PlayerName().c_str());
                                    AbilityData.TimesLogged++;
                                }
                                
                                return false;
                            }
                        }
                    }
                }
                else // burst, probably
                {
                    float FiringRate = GetBurstFiringRate(CurrentWeapon);

                    if (FiringRate > 0.0)
                    {
                        float TimeShotDelta = (*GWorld)->TimeSeconds() - AbilityData.LastShotTime;
                        AbilityData.LastShotTime = (*GWorld)->TimeSeconds();

                        if (TimeShotDelta < std::fmaxf(std::fminf(1.0 / FiringRate - 0.1, 1.0 / FiringRate * 0.5), 0.0))
                        {
                            // Log only first 10 times
                            if (AbilityData.TimesLogged < 10)
                            {
                                spdlog::error(L"Player '{}' has insufficient TimeShotDelta [Value: {}, ExpectedValue: {}, CartridgePerFire: {}]", Controller->MyPlayerInfo()->PlayerName().c_str(), TimeShotDelta, std::fmaxf(std::fminf(1.0 / FiringRate - 0.1, 1.0 / FiringRate * 0.5), 0.0), WeaponStats->CartridgePerFire());
                                AbilityData.TimesLogged++;
                            }
                        
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

float anticheat_component::GetBurstFiringRate(AFortWeapon* FortWeapon)
{
    if (anticheat_component_offsets::GetBurstFiringRateOffset > 0)
    {
        return reinterpret_cast<float (*)(AFortWeapon*)>(FortWeapon->VTable[anticheat_component_offsets::GetBurstFiringRateOffset])(FortWeapon);
    }
    
    return 0.0;
}

bool anticheat_component::ValidateTargetDataFrequency(FRangedAbilityData& AbilityData, UFortGameplayAbility_RangedWeapon* Ability, UWorld* World, float FiringRatePerSecond, float OptimalTimeDelta, float CustomTimeDilation)
{
    float TimeShotDelta;
    
    if (AbilityData.TargetDataTimeCount)
    {
        TimeShotDelta = World->TimeSeconds() - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex] * CustomTimeDilation;
    }
    else
    {
        TimeShotDelta = 1000000.0;
    }

    if (AbilityData.TargetDataTimeCount + 1 < 8)
    {
        AbilityData.TargetDataTimeCount++;
    }
    else
    {
        AbilityData.TargetDataTimeCount = 8;
    }

    AbilityData.TargetDataTimeIndex = AbilityData.TargetDataTimeIndex + 1 & 7;

    AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex] = World->TimeSeconds();

    if (TimeShotDelta < OptimalTimeDelta)
    {
        return false;
    }

    if (AbilityData.TargetDataTimeCount != 8)
    {
        return true;
    }

    float TotalTimeShotDelta = 0.0;
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 2 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 1 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 3 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 2 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 4 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 3 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 3 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex + 4 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 2 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 3 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 1 & 7] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 2 & 7];
    TotalTimeShotDelta += AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex] - AbilityData.TargetDataTime[AbilityData.TargetDataTimeIndex - 1 & 7];
    TotalTimeShotDelta *= 0.14285715;

    if (TotalTimeShotDelta < std::fmaxf(FiringRatePerSecond - 0.1, FiringRatePerSecond * 0.75))
    {
        return false;
    }
    
    return true;
}

float anticheat_component::GetFiringRate(AFortWeapon* FortWeapon)
{
    if (anticheat_component_offsets::GetFiringRateOffset > 0)
    {
        return reinterpret_cast<float (*)(AFortWeapon*)>(FortWeapon->VTable[anticheat_component_offsets::GetFiringRateOffset])(FortWeapon);
    }
    
    return 0.0;
}

void anticheat_component_hooks::ServerNotifyPawnHit_Hook(AFortProjectileAthena* Self, FFrame& StackFrame)
{
    GET_PARAMETER(FHitResult, Hit);

    // Get the player controller
    AFortPlayerController* Controller = static_cast<AFortPlayerPawn*>(Self->GetOwner()->GetOwner())->Controller();

    if (Controller)
    {
        if (Hit.Actor().Get() && Hit.Actor().Get()->IsA(APlayerPawn_Athena_C::StaticClass()))
        {
            // Check if TraceEnd and the bullet's location on the server are different
            FVector CurrentLoc = Self->K2_GetActorLocation();
            float Difference_X = sqrtf((Hit.TraceEnd().X - CurrentLoc.X) * (Hit.TraceEnd().X - CurrentLoc.X));
            float Difference_Y = sqrtf((Hit.TraceEnd().Y - CurrentLoc.Y) * (Hit.TraceEnd().Y - CurrentLoc.Y));

            if (Difference_X >= 5000 ||
                Difference_Y >= 5000)
            {
                // Log
                spdlog::warn(L"'{}' has hit unlikely bullet. [X axis distance: {}, Y axis distance: {}]", Controller->MyPlayerInfo()->PlayerName().c_str(), Difference_X, Difference_Y);

                // Register invalid hit
                anticheat_component::IllegalSniperActionsCount[Controller]++;
            
                // Check amount of illegal actions
                if (anticheat_component::IllegalSniperActionsCount[Controller] >= 7)
                {
                    // Perform a ban
                    if (auto account_id = player_component::get_account_id(Controller); account_id.size())
                    {
                        gameserver_client::Client->SendBan(account_id);
                    }
                }
            
                return;
            }
        }
        
        return ServerNotifyPawnHit(Self, StackFrame);
    }
}