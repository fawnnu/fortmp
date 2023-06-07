#include "include.hpp"
#include "challenges_component.hpp"

#include "player_component.hpp"
#include "team_component.hpp"

challenges_component::challenges_component()
{
       
}

void challenges_component::SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer)
{
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:chest", parameters);
        }
    }
}

void challenges_component::SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer)
{
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:ammo", parameters);
        }
    }
}

void challenges_component::SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self)
{
    if (OwnerPC && Self)
    {
        std::string account_id = player_component::get_account_id(OwnerPC);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(Self->K2_GetActorLocation().X)},
                {"y", std::to_string(Self->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:landing", parameters);
        }
    }
}

void challenges_component::SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama)
{
    if (InteractingController && Llama)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(Llama->K2_GetActorLocation().X)},
                {"y", std::to_string(Llama->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:llama", parameters);
        }
    }
}

void challenges_component::SendSummary(AFortPlayerController* PlayerController, int Placement)
{
    if (PlayerController)
    {
        std::string account_id = player_component::get_account_id(PlayerController);
        int DamageDealt = 0;

        if (player_component::DamageDealtMap.contains(PlayerController))
        {
            DamageDealt = player_component::DamageDealtMap[PlayerController];
        }

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                    {"placement", std::to_string(Placement)},
                    {"damage", std::to_string(DamageDealt)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "summary", parameters);

            // Log
            if (!memory_manager::is_bad_read_pointer(PlayerController->MyPlayerInfo()) && PlayerController->MyPlayerInfo())
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::wstring wAccount_id = converter.from_bytes(account_id);
                
                spdlog::info(L"Sent summary event for {} [ACCOUNT_ID: {}, PLACEMENT: {}, DAMAGEDEALT: {}]", PlayerController->MyPlayerInfo()->PlayerName().c_str(), wAccount_id.c_str(), Placement, DamageDealt);
            }
        }
    }
}