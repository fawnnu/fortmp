#pragma once

class player_component
{
public:
    player_component();

    static AFortPlayerPawn* create_player(AFortPlayerController* player_controller, FTransform spawn_transform);
};

namespace player_component_hooks
{
    inline void (*SetClientLoginState)(UNetConnection* NetConnection, int State);

    // Called when a player joins the server
    void SetClientLoginState_Hook(UNetConnection* NetConnection, int State);
}
