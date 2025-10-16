#include "include.hpp"
#include "anticheat_component.hpp"

#include "creative_component.hpp"
#include "log_component.hpp"
#include "looting_component.hpp"
#include "moderation_component.hpp"
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

bool anticheat_component::ValidateBuildingPlacement(AFortPlayerControllerAthena* Player, FVector BuildingLocation)
{
    if (!memory_manager::is_bad_read_pointer(Player) && Player)
    {
        if (Player->IsA(AFortPlayerControllerAthena::StaticClass()))
        {
            if (!memory_manager::is_bad_read_pointer(Player->Pawn()) && Player->Pawn())
            {
                if (Player->Pawn()->IsA(AFortPlayerPawnAthena::StaticClass()))
                {
                    // check location
                    AFortPlayerPawnAthena* Pawn = static_cast<AFortPlayerPawnAthena*>(Player->Pawn());

                    // Get location
                    FVector PlayerLocation = Pawn->K2_GetActorLocation();

                    // Get distance from requested location
                    const float Distance = sqrtf((PlayerLocation.X - BuildingLocation.X) * (PlayerLocation.X - BuildingLocation.X) +
                                                (PlayerLocation.Y - BuildingLocation.Y) * (PlayerLocation.Y - BuildingLocation.Y) +
                                                (PlayerLocation.Z - BuildingLocation.Z) * (PlayerLocation.Z - BuildingLocation.Z));
                        
                    // Check the distance, if it's more than 30 meters count as illegal
                    if (Distance > 5000.0)
                    {
                        Player->EraPlayerInfo().IllegalBuildingPlacements++;
                        spdlog::error("Player attempting illegal building placement. Aborting. Count: {}", Player->EraPlayerInfo().IllegalBuildingPlacements);

                        return false;
                    }

                    // Creative checks!
                    if (creative_component::is_creative_enabled())
                    {
                        if (Player->Cast<AFortPlayerControllerAthena>())
                        {
                            if (auto Minigame = Player->GetMinigame())
                            {
                                if (Minigame->IsPlayerInOurVolume(Player->PlayerState()))
                                {
                                    /*
                                     * Only allow building within the minigame volume (island)
                                     */
                                    return true;
                                }
                            }
                        }

                        return false;
                    }
                    
                    return true;
                }
            }
        }
    }

    return false;
}

bool anticheat_component::ValidateInteractionDistance(AActor* Sender, AActor* Receiver)
{
    auto SenderLoc = Sender->K2_GetActorLocation();
    auto ReceiverLoc = Receiver->K2_GetActorLocation();
    const float DistanceXYZ = sqrtf((SenderLoc.X - ReceiverLoc.X) * (SenderLoc.X - ReceiverLoc.X) +
            (SenderLoc.Y - ReceiverLoc.Y) * (SenderLoc.Y - ReceiverLoc.Y) +
            (SenderLoc.Z - ReceiverLoc.Z) * (SenderLoc.Z - ReceiverLoc.Z));

    if (DistanceXYZ > 1500)
    {
        return false;
    }
    
    return true;
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


// TODO: Move to client
bool anticheat_component::ValidateShot(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility,
    FGameplayAbilityTargetDataHandle& InData)
{
    if (FCollisionQueryParams__Ctor &&
        FCollisionQueryParams__AddIgnoredActors &&
        LineTraceMultiByChannel)
    {
        if (InData.GetData().Num() == 1)
        {
            if (RangedWeaponAbility)
            {
                AFortWeapon* Weapon = RangedWeaponAbility->CurrentWeapon();
                UFortWeaponRangedItemDefinition* WeaponRangedData = reinterpret_cast<UFortWeaponRangedItemDefinition*>(Weapon->WeaponData());
            
                if (WeaponRangedData->IsA(UFortWeaponRangedItemDefinition::StaticClass()) && !Weapon->IsProjectileWeapon())
                {
                    if (FFortRangedWeaponStats* WeaponStats = reinterpret_cast<FFortRangedWeaponStats*>(looting_component::get_weapon_stats(WeaponRangedData)))
                    {
                        if (WeaponStats->SweepRadius() == 0.0)
                        {
                            FVector TraceStart = InData.GetSingleData().Get()->HitResult().TraceStart();
                            FVector TraceEnd = InData.GetSingleData().Get()->HitResult().TraceEnd();
                            FVector& ClientImpactPoint = InData.GetSingleData().Get()->HitResult().ImpactPoint();

                            // Result
                            TArray<FHitResult> HitResults;

                            // Params
                            static FName WeaponTrace = FName(TEXT("WeaponTrace"));
                            FCollisionResponseParams CollisionResponseParams = FCollisionResponseParams();
                            FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams(WeaponTrace, true, RangedWeaponAbility->GetAvatarActorFromActorInfo());

                            // Line trace
                            (*GWorld)->LineTraceMultiByChannel(HitResults, TraceStart, TraceEnd, FCC_Trace_Weapon, CollisionQueryParams, CollisionResponseParams);

                            // Check results
                            if (HitResults.Num() == 1)
                            {
                                FVector& ServerImpactPoint = HitResults[0].ImpactPoint();

                                // Round
                                ServerImpactPoint.X = round(ServerImpactPoint.X);
                                ServerImpactPoint.Y = round(ServerImpactPoint.Y);
                                ServerImpactPoint.Z = round(ServerImpactPoint.Z);

                                // Compare
                                if (ServerImpactPoint.X != ClientImpactPoint.X ||
                                    ServerImpactPoint.Y != ClientImpactPoint.Y ||
                                    ServerImpactPoint.Z != ClientImpactPoint.Z)
                                {
                                    const float Distance = sqrtf((ServerImpactPoint.X - ClientImpactPoint.X) * (ServerImpactPoint.X - ClientImpactPoint.X) +
                                                                 (ServerImpactPoint.Y - ClientImpactPoint.Y) * (ServerImpactPoint.Y - ClientImpactPoint.Y) +
                                                                 (ServerImpactPoint.Z - ClientImpactPoint.Z) * (ServerImpactPoint.Z - ClientImpactPoint.Z));
                        
                                    // Check the distance, if it's less than 10cm, allow the client sent values.
                                    if (Distance > 10.0)
                                    {
                                        if (log_component::enable_debug_logs)
                                        {
                                            AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(Weapon->GetOwner());
                                            AFortPlayerController* Controller = Pawn->Controller();
                                            
                                            spdlog::warn(L"player_component_hooks::OnTargetDataReadyCallback_Hook: Player '{}' sent mismatched values.", Controller->MyPlayerInfo()->PlayerName().c_str());
                                            spdlog::warn(L"player_component_hooks::OnTargetDataReadyCallback_Hook: Server Impact Point: {} {} {}.", ServerImpactPoint.X, ServerImpactPoint.Y, ServerImpactPoint.Z);
                                            spdlog::warn(L"player_component_hooks::OnTargetDataReadyCallback_Hook: Client Impact Point: {} {} {}.", ClientImpactPoint.X, ClientImpactPoint.Y, ClientImpactPoint.Z);
                                        }

                                        // Override shot
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

bool anticheat_component::CheckForBulletTP(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility,
    FGameplayAbilityTargetDataHandle& InData, float* OutDistance)
{
    // Any normal gun, ignoring shotguns
    if (InData.GetData().Num() == 1)
    {
        auto TraceStartVector = InData.GetSingleData().Get()->HitResult().TraceStart();
        auto CurrentWeaponLocation = RangedWeaponAbility->CurrentWeapon()->K2_GetActorLocation();
        const float TraceStartToWeaponDistance = sqrtf((TraceStartVector.X - CurrentWeaponLocation.X) * (TraceStartVector.X - CurrentWeaponLocation.X) +
                                            (TraceStartVector.Y - CurrentWeaponLocation.Y) * (TraceStartVector.Y - CurrentWeaponLocation.Y) +
                                            (TraceStartVector.Z - CurrentWeaponLocation.Z) * (TraceStartVector.Z - CurrentWeaponLocation.Z));

        // The distance should NEVER be more than 7.5 meters, in any scenario!
        if (TraceStartToWeaponDistance >= 750.0)
        {
            if (OutDistance)
            {
                *OutDistance = TraceStartToWeaponDistance;
            }
            
            // Blank the shot
            return false;
        }
    }
    else
    {
        for (uint8 i = 0; i < InData.GetData().Num() && i < 255; ++i)
        {
            if (FFortGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = InData.Get(i))
            {
                auto TraceStartVector = SingleTargetHit->HitResult().TraceStart();
                auto CurrentWeaponLocation = RangedWeaponAbility->CurrentWeapon()->K2_GetActorLocation();
                const float TraceStartToWeaponDistance = sqrtf((TraceStartVector.X - CurrentWeaponLocation.X) * (TraceStartVector.X - CurrentWeaponLocation.X) +
                                            (TraceStartVector.Y - CurrentWeaponLocation.Y) * (TraceStartVector.Y - CurrentWeaponLocation.Y) +
                                            (TraceStartVector.Z - CurrentWeaponLocation.Z) * (TraceStartVector.Z - CurrentWeaponLocation.Z));

                // The distance should NEVER be more than 7.5 meters, in any scenario!
                if (TraceStartToWeaponDistance >= 750.0)
                {
                    if (OutDistance)
                    {
                        *OutDistance = TraceStartToWeaponDistance;
                    }
            
                    // Blank the shot
                    return false;
                }
            }
        }
    }

    return true;
}

bool anticheat_component::CheckShotTrajectory(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility,
    FGameplayAbilityTargetDataHandle& InData)
{
    // Any normal gun, ignoring shotguns
    if (InData.GetData().Num() == 1)
    {
        // If we didn't hit a pawn, trust the shot
        if (AActor* Actor = InData.GetSingleData().Get()->HitResult().Actor().Get())
        {
            if (Actor->Class != APlayerPawn_Athena_C::StaticClass())
            {
                return true;
            }
        }
        
        TArray<FHitResult> HitResults;

        // Params
        static FName WeaponTrace = FName(TEXT("WeaponTrace"));
        FCollisionResponseParams CollisionResponseParams = FCollisionResponseParams();
        FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams(WeaponTrace, true, RangedWeaponAbility->GetAvatarActorFromActorInfo());
        
        // Line trace
        if ((*GWorld)->LineTraceMultiByChannel(HitResults, InData.GetSingleData().Get()->HitResult().TraceStart(), InData.GetSingleData().Get()->HitResult().ImpactPoint(), FCC_Trace_Weapon, CollisionQueryParams, CollisionResponseParams))
        {
            if (HitResults.Num() == 1)
            {
                if (HitResults[0].Actor().Get() && HitResults[0].Actor().Get()->Class != APlayerPawn_Athena_C::StaticClass())
                {
                    if (log_component::enable_debug_logs)
                    {
                        if (RangedWeaponAbility->CurrentWeapon() &&
                            RangedWeaponAbility->CurrentWeapon()->Owner())
                        {
                            if (AFortPlayerPawnAthena* Pawn = RangedWeaponAbility->CurrentWeapon()->Owner()->Cast<AFortPlayerPawnAthena>())
                            {
                                spdlog::warn(L"anticheat_component::CheckShotTrajectory: Player's ['{}'] shot impacted actor '{}'.", Pawn->Controller()->MyPlayerInfo()->PlayerName().c_str(), HitResults[0].Actor().Get()->GetName().c_str());
                            }
                        }
                    }

                    return false;
                }
            }
        }
    }
    else
    {
        for (uint8 i = 0; i < InData.GetData().Num() && i < 255; ++i)
        {
            if (FFortGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = InData.Get(i))
            {
                // If we didn't hit a pawn, trust the shot
                if (AActor* Actor = SingleTargetHit->HitResult().Actor().Get())
                {
                    if (Actor->Class != APlayerPawn_Athena_C::StaticClass())
                    {
                        continue;
                    }
                }
        
                TArray<FHitResult> HitResults;

                // Params
                static FName WeaponTrace = FName(TEXT("WeaponTrace"));
                FCollisionResponseParams CollisionResponseParams = FCollisionResponseParams();
                FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams(WeaponTrace, true, RangedWeaponAbility->GetAvatarActorFromActorInfo());
        
                // Line trace
                if ((*GWorld)->LineTraceMultiByChannel(HitResults, SingleTargetHit->HitResult().TraceStart(), SingleTargetHit->HitResult().ImpactPoint(), FCC_Trace_Weapon, CollisionQueryParams, CollisionResponseParams))
                {
                    if (HitResults.Num() == 1)
                    {
                        if (HitResults[0].Actor().Get() && HitResults[0].Actor().Get() != APlayerPawn_Athena_C::StaticClass())
                        {
                            if (log_component::enable_debug_logs)
                            {
                                if (RangedWeaponAbility->CurrentWeapon() &&
                                    RangedWeaponAbility->CurrentWeapon()->Owner())
                                {
                                    if (AFortPlayerPawnAthena* Pawn = RangedWeaponAbility->CurrentWeapon()->Owner()->Cast<AFortPlayerPawnAthena>())
                                    {
                                        spdlog::warn(L"anticheat_component::CheckShotTrajectory: Player's ['{}'] shot impacted actor '{}'.", Pawn->Controller()->MyPlayerInfo()->PlayerName().c_str(), HitResults[0].Actor().Get()->GetName().c_str());
                                    }
                                }
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
    AFortPlayerController* Controller = nullptr;
    
    if(Self)
    {
        if(Self->GetOwner())
        {
            if(Self->GetOwner()->GetOwner())
            {
                if(auto pawn = static_cast<AFortPlayerPawn*>(Self->GetOwner()->GetOwner()))
                {
                    Controller = pawn->Controller();
                }
            }
        }
    }
    
    if (Controller)
    {
        if (Hit.Actor().Get() && Hit.Actor().Get()->IsA(APlayerPawn_Athena_C::StaticClass()))
        {
            // Don't run these checks on less than 50m hits
            if (Self->GetDistanceTo(Hit.Actor().Get()) > 5000)
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
        }
        
        return ServerNotifyPawnHit(Self, StackFrame);
    }
}