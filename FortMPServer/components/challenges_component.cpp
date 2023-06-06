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