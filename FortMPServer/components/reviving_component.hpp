#pragma once

class reviving_component
{
public:
    reviving_component();

    static void revive_player(AFortPlayerController* player_controller, AFortPlayerPawn* player_to_revive, bool bForceRevive = false);
};