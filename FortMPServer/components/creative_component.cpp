#include "include.hpp"
#include "creative_component.hpp"

creative_component::creative_component()
{
}

void creative_component::handle_player_join(UNetConnection* NetConnection)
{
    // Assign portal to player
    assign_portal_to_player(NetConnection);
}

void creative_component::assign_portal_to_player(UNetConnection* NetConnection)
{
    AFortCreativePortalManager* creative_portal_manager = (*GWorld)->GameState()->CreativePortalManager();

    if(creative_portal_manager != nullptr)
    {
        AFortAthenaCreativePortal* creative_portal = creative_portal_manager->AvailablePortals()[0];

        AFortPlayerControllerAthena* player_controller_athena = reinterpret_cast<AFortPlayerControllerAthena*>(NetConnection->PlayerController());
        
        /* @ Setting the portal owner ship.
         * @ Allows the player to interact in the portal and teleport through it once the stripped functions are re-implemented.*/
        creative_portal->OwningPlayer() = NetConnection->PlayerID();
        player_controller_athena->OwnedPortal() = creative_portal;
        creative_portal->CachedOwningPlayerState() = NetConnection->PlayerController()->PlayerState();
        
        creative_portal->SetOwner(player_controller_athena);
        creative_portal->OwningPlayerChanged();
        creative_portal->OnRep_OwningPlayer();
        
        // Mark player as ready to use the portal
        creative_portal->PlayersReady().Add(NetConnection->PlayerID());
        creative_portal->OnRep_PlayersReady();
        
        // Open the portal
        creative_portal->bPortalOpen() = true;
        creative_portal->bDisallowPortalInteract() = false;
        creative_portal->PortalOpenChanged();

        // Mark portal as used so it doesn't get assigned to another player
        creative_portal_manager->MarkPortalUsed(creative_portal);
    }
}