#pragma once

class player_component
{
public:
    player_component();

    static AFortPlayerPawn* create_player(AFortPlayerController* player_controller, FTransform spawn_transform);
};

namespace player_component_hooks
{
    // Called when a player joins the server
    DEFINE_FUNCTION_HOOK(void, SetClientLoginState, (UNetConnection* NetConnection, int State));

    // Called to override the hero type
    DEFINE_FUNCTION_HOOK(UObject*, GetHeroDefinition, (UObject* Hero));
    
}
