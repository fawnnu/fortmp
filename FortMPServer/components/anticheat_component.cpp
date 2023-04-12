#include "include.hpp"
#include "anticheat_component.hpp"

#include "player_component.hpp"

anticheat_component::anticheat_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(anticheat_component_hooks::ServerNotifyPawnHit, TEXT("/Script/FortniteGame.FortProjectileAthena.ServerNotifyPawnHit"), anticheat_component_hooks::ServerNotifyPawnHit_Hook)
    
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
                anticheat_component::IllegalActionsCount[Controller]++;
            
                // Check amount of illegal actions
                if (anticheat_component::IllegalActionsCount[Controller] >= 3)
                {
                    // Perform a ban
                    for (auto [account_id, player_controller] : player_component::Players)
                    {
                        if (!memory_manager::is_bad_read_pointer(player_controller) && player_controller)
                        {
                            if (player_controller == Controller)
                            {
                                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                            
                                gameserver_client::Client->SendBan(converter.to_bytes(account_id));
                            
                                break;
                            }
                        }
                    }
                }
            
                return;
            }
        }
        
        return ServerNotifyPawnHit(Self, StackFrame);
    }
}
