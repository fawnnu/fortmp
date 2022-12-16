#pragma once

class team_component
{
public:
    team_component();

    static void set_team(AFortPlayerController* player_controller, uint8_t team_index);
    static uint8_t find_team(AFortPlayerController* player_controller);
    static int get_team_size();
    static int get_default_first_team();
    static int get_default_last_team();
};

namespace team_component_hooks
{
    // Called when the game tries to select a team for the player
    DEFINE_FUNCTION_HOOK(uint8_t, PickTeam, (AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController));
}