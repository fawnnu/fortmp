#pragma once

struct era_team_info
{
    era_team_info() = default;
    
    std::vector<std::wstring> TeamMembers;
    std::vector<AFortPlayerController*> TeamMembersControllers;
    bool fill;
    uint8_t preferred_team;
    uint8_t preferred_main_team;
};

class team_component
{
public:
    team_component();

    static uint8_t PickTeam_Implementation(uint8_t InTeamIndex, AFortPlayerController* FortPlayerController);
    static void InitializeTeams();
    static void Login(AFortPlayerController* player_controller, const FString& Options);

    static struct era_team* find_free_team(AFortPlayerController* player_controller);
    static era_team* find_main_team(AFortPlayerController* player_controller);
    static void set_team(AFortPlayerController* player_controller, uint8_t team_index, uint8_t squad_id);
    static AFortTeamInfo* find_team(AFortPlayerController* player_controller);
    static era_team* find_era_team(AFortPlayerController* player_controller, bool bTeamIndex);
    static int get_squad_size();
    static int get_team_size();
    static int get_default_first_team();
    static int get_default_last_team();
    static int get_alive_teams();

    static inline int max_team_size_override;
    static inline std::map<std::wstring, era_team_info> TeamsMap;
    static inline std::vector<era_team*> TeamsList;
    static inline std::vector<era_team*> MainTeamsList;
};

enum class era_team_status
{
    empty,
    available_for_fill,
    available_for_teammates,
    not_available
};

struct era_team
{
    era_team()
    {
        status = era_team_status::empty;
        bound_team_index = 0;
        bound_era_team_info = new era_team_info;
        players_count = 0;
        team_component::TeamsList.push_back(this);
    }

    era_team(uint8_t in_team_index, bool bIsMainTeam)
    {
        status = era_team_status::empty;
        bound_team_index = in_team_index;
        bound_era_team_info = new era_team_info;
        players_count = 0;
        bIsMainTeam ? team_component::MainTeamsList.push_back(this) : team_component::TeamsList.push_back(this);
    }

    int& get_team_members()
    {
        return players_count;
    }

    uint8_t& get_team_index()
    {
        return bound_team_index;
    }

    bool operator<(era_team& cmp)
    {
        return players_count < cmp.players_count;
    }

    int players_count;
    era_team_status status;
    uint8_t bound_team_index;
    era_team_info* bound_era_team_info;
};

namespace team_component_hooks
{
    // Called when the game tries to select a team for the player
    DEFINE_FUNCTION_HOOK(uint8_t, PickTeam, (AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController));
}