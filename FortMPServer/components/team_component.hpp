#pragma once

class team_component
{
public:
    team_component();

    static void Login(AFortPlayerController* player_controller, const FString& Options);

    static void set_team(AFortPlayerController* player_controller, uint8_t team_index);
    static AFortTeamInfo* find_team(AFortPlayerController* player_controller);
    static int get_team_size();
    static int get_default_first_team();
    static int get_default_last_team();
    static uint8_t find_free_team(bool bMustBeEmpty = false, bool bMustBeReserved = false, std::wstring team_code = L"");

    static inline std::map<std::wstring, std::vector<AFortPlayerController*>> team_codes_map;
    static inline std::map<std::wstring, uint8_t> reserved_teams;
};

namespace team_component_hooks
{
    // Called when the game tries to select a team for the player
    DEFINE_FUNCTION_HOOK(uint8_t, PickTeam, (AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController));
}