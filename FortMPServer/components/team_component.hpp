#pragma once

struct era_team_info
{
    era_team_info() = default;
    
    std::vector<std::wstring> TeamMembers;
    std::vector<AFortPlayerController*> TeamMembersControllers;
    bool fill;
    uint8_t preferred_team;
};

class team_component
{
public:
    team_component();

    static void InitializeTeams();
    static void Login(AFortPlayerController* player_controller, const FString& Options);

    static struct era_team* find_free_team(AFortPlayerController* player_controller);
    static void set_team(AFortPlayerController* player_controller, uint8_t team_index);
    static AFortTeamInfo* find_team(AFortPlayerController* player_controller);
    static era_team* find_era_team(AFortPlayerController* player_controller);
    static int get_team_size();
    static int get_default_first_team();
    static int get_default_last_team();

    static inline int max_team_size_override;
    static inline std::map<std::wstring, era_team_info> TeamsMap;
    static inline std::vector<era_team*> TeamsList;
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
        bound_team_info = nullptr;
        bound_era_team_id = L"";
        players_count = 0;
        team_component::TeamsList.push_back(this);
    }

    era_team(AFortTeamInfo* in_team_info)
    {
        status = era_team_status::empty;
        bound_team_info = in_team_info;
        bound_era_team_id = L"";
        players_count = 0;
        team_component::TeamsList.push_back(this);
    }

    int& get_team_members()
    {
        return players_count;
    }

    uint8_t& get_team_index()
    {
        return bound_team_info->Team();
    }

    int players_count;
    era_team_status status;
    AFortTeamInfo* bound_team_info;
    std::wstring bound_era_team_id;
};

namespace team_component_hooks
{
    // Called when the game tries to select a team for the player
    DEFINE_FUNCTION_HOOK(uint8_t, PickTeam, (AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController));
}