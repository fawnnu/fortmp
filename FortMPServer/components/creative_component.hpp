#pragma once

class creative_component
{
public:

    // Init
    creative_component();

    // Assign a portal to the joining player
    static void assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex);
    
    // Handle everything that has to be done once a player joins the match
    static void handle_player_join(UNetConnection* NetConnection);
};

namespace creative_component_hooks
{
    // Called when a player enters a creative portal
    DEFINE_FUNCTION_HOOK(void, TeleportPlayerToLinkedVolume, (AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags));

    // Called when
    DEFINE_FUNCTION_HOOK(void, ServerGiveCreativeItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
}