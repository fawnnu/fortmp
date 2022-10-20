#pragma once

class creative_component
{
public:

    // Init
    creative_component();

    // Assign a portal to the joining player
    static void assign_portal_to_player(UNetConnection* NetConnection);
    
    // Handle everything that has to be done once a player joins the match
    static void handle_player_join(UNetConnection* NetConnection);
};

namespace creative_component_hooks
{
}