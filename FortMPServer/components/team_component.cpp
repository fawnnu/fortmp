#include "include.hpp"
#include "team_component.hpp"

#include "loader_component.hpp"

team_component::team_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (GetEngineVersionEasy() > 419)
    {
        REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%d], actual: [%d]"), team_component_hooks::PickTeam_Hook);
        REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%s], actual: [%s]"), team_component_hooks::PickTeam_Hook);
    }
    
    DetourTransactionCommit();
}

void team_component::set_team(AFortPlayerController* player_controller, uint8_t team_index)
{
    // Set team
    player_controller->PlayerState()->TeamIndex() = team_index;
    player_controller->PlayerState()->SquadId() = team_index;
    
    if (FGameMemberInfo::GetClass())
    {
        FGameMemberInfo game_member_info {};

        // Setup game member info
        game_member_info.TeamIndex = team_index;
        game_member_info.SquadId = team_index;
        game_member_info.MemberUniqueId = player_controller->NetConnection()->PlayerID();

        // Add to game member list
        TArrayManager<FGameMemberInfo>(&(*GWorld)->GameState()->GameMemberInfoArray().Members(), FGameMemberInfo::GetStructSize()).Add(&game_member_info);

        // Mark last item for replication
        (*GWorld)->GameState()->GameMemberInfoArray().MarkItemDirty((*GWorld)->GameState()->GameMemberInfoArray().Members()[(*GWorld)->GameState()->GameMemberInfoArray().Members().Num() - 1]);
    }
}

uint8_t team_component::find_team(AFortPlayerController* player_controller)
{
    // List of all teams
    TArray<AFortTeamInfo*> all_teams = (*GWorld)->GameState()->Teams();

    for (AFortTeamInfo* team_info : all_teams)
    {
        for (AController* team_member : team_info->TeamMembers())
        {
            if (team_member == player_controller)
            {
                // Return found team
                return team_info->Team();
            }
        }
    }

    return 0;
}

int team_component::get_team_size()
{
    // Get current playlist
    UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
        (*GWorld)->GameState()->CurrentPlaylistData() : (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist();

    return current_playlist->MaxTeamSize();
}

uint8_t team_component_hooks::PickTeam_Hook(AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController)
{
    if (loader_component::is_server_ready)
    {
        // List of all teams
        TArray<AFortTeamInfo*> all_teams = (*GWorld)->GameState()->Teams();

        for (int team_idx = 3; team_idx < all_teams.Num(); team_idx++)
        {
            if (all_teams[team_idx]->TeamMembers().Num() < team_component::get_team_size())
            {
                // Add player to team list
                TArrayManager(&all_teams[team_idx]->TeamMembers(), sizeof(UObject*)).Add(&PlayerController);

                // Return chosen team
                return team_idx;
            }
        }
    }
    
    return PickTeam(FortGameMode, InTeamIndex, PlayerController);
}