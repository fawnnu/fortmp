#include "include.hpp"
#include "creative_component.hpp"

#include "inventory_component.hpp"

creative_component::creative_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", creative_component_hooks::TeleportPlayerToLinkedVolume_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerGiveCreativeItem, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem", creative_component_hooks::ServerGiveCreativeItem_Hook);
    
    DetourTransactionCommit();
}

void creative_component::handle_player_join(UNetConnection* NetConnection)
{
    /* @ Assign portal to player
     * @ TODO: Use portal index from creative player start */
    assign_portal_to_player(NetConnection, 0);
}

void creative_component::assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex)
{
    AFortCreativePortalManager* creative_portal_manager = (*GWorld)->GameState()->CreativePortalManager();
    AFortVolumeManager* volume_manager = (*GWorld)->GameState()->VolumeManager();

    if(creative_portal_manager != nullptr)
    {
        AFortAthenaCreativePortal* creative_portal = creative_portal_manager->AvailablePortals()[PortalIndex];

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

void creative_component_hooks::TeleportPlayerToLinkedVolume_Hook(AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags)
{
    PlayerPawn->Call(L"K2_TeleportTo", Portal->LinkedVolume()->K2_GetActorLocation(), FRotator{});
    PlayerPawn->Call(L"TeleportToSkyDive", 8000);
    
    Portal->Call(L"NotifyTeleportedPlayerPawn", PlayerPawn, false);
    Portal->Call(L"OnPlayerPawnTeleported", PlayerPawn, false);
}

void creative_component_hooks::ServerGiveCreativeItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FFortItemEntry, CreativeItem);

    if(CreativeItem.Count() > 0)
    {
        inventory_component::add_item(FortPlayerController, CreativeItem.ItemDefinition(), CreativeItem.Count(), CreativeItem.LoadedAmmo());   
    }
    
    return ServerGiveCreativeItem(FortPlayerController, StackFrame);
}