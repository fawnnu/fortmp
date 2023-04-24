#include "include.hpp"
#include "anticheat_component.hpp"

#include "player_component.hpp"

anticheat_component::anticheat_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(anticheat_component_hooks::ServerNotifyPawnHit, TEXT("/Script/FortniteGame.FortProjectileAthena.ServerNotifyPawnHit"), anticheat_component_hooks::ServerNotifyPawnHit_Hook)
    REGISTER_FUNCTION_DETOUR(anticheat_component_hooks::OnTargetDataReadyCallback, memory_manager::find_function_by_string(L"STAT_FortGA_Ranged_OnTargetDataReadyCB"), anticheat_component_hooks::OnTargetDataReadyCallback_Hook);
    
    DetourTransactionCommit();
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
                if (anticheat_component::IllegalSniperActionsCount[Controller] >= 3)
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

//TODO: This code has one big flaw, if the ping itself is higher than the possible fire rate, anyone with that ping will simply bypass the checks
__int64 anticheat_component_hooks::OnTargetDataReadyCallback_Hook(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
    if (RangedWeaponAbility->IsA(UFortGameplayAbility_RangedWeapon::StaticClass()))
    {
        AFortWeapon* CurrentWeapon = RangedWeaponAbility->CurrentWeapon();

        // Get firing player
        AFortPlayerController* Controller = RangedWeaponAbility->GetActivatingPawn()->Controller();
    
        if (anticheat_component::LastFireTime[CurrentWeapon] == 0.0f)
        {
            // Update for initial weapon fire
            anticheat_component::LastFireTime[CurrentWeapon] = (*GWorld)->TimeSeconds();
        }
        else
        {
            const float TimeSinceLastFired = (*GWorld)->TimeSeconds() - anticheat_component::LastFireTime[CurrentWeapon];

            // Get ping and include it in the calculation. NOTE: The value stored in "Ping" is divided by 4 for performance reasons
            float RealPingInSeconds = (Controller->PlayerState()->Ping() * 4) / 1000.f;
        
            if ((TimeSinceLastFired + 0.035f + RealPingInSeconds) < (1.0f / CurrentWeapon->GetFiringRate()))
            {
                // Log
                spdlog::warn(L"Player '{}' has exceeded fire rate (TimeSinceLastFired: {}, FireRate: {})", Controller->MyPlayerInfo()->PlayerName().c_str(), TimeSinceLastFired + 0.035f + RealPingInSeconds, (1.0f / CurrentWeapon->GetFiringRate()));
            
                // Register invalid shot
                anticheat_component::IllegalWeaponActionsCount[CurrentWeapon]++;

                if (anticheat_component::IllegalWeaponActionsCount[CurrentWeapon] >= 5)
                {
                    // Kick the player
                    Controller->ClientReturnToMainMenu(TEXT(""));
                }
            }

            anticheat_component::LastFireTime[CurrentWeapon] = (*GWorld)->TimeSeconds();
        }
    }
    
    return OnTargetDataReadyCallback(RangedWeaponAbility, InData, ApplicationTag);
}