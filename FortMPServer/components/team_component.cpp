#include "include.hpp"
#include "team_component.hpp"

#include "loader_component.hpp"

team_component::team_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%d], actual: [%d]"), team_component_hooks::PickTeam_Hook);
    REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%s], actual: [%s]"), team_component_hooks::PickTeam_Hook);
        
    DetourTransactionCommit();
}

void team_component::set_team(AFortPlayerController* player_controller, uint8_t team_index)
{
    // Set team
    player_controller->PlayerState()->TeamIndex() = team_index;

    if (STATIC_HASPROPERTY(player_controller->PlayerState(), SquadId))
    {
        player_controller->PlayerState()->SquadId() = team_index;
    }
    
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

AFortTeamInfo* team_component::find_team(AFortPlayerController* player_controller)
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
                return team_info;
            }
        }
    }

    return nullptr;
}

int team_component::get_team_size()
{
    constexpr int max_team_size_override = 1;
    
    // Get current playlist
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();

    return current_playlist ? current_playlist->MaxTeamSize() : max_team_size_override;
}

int team_component::get_default_first_team()
{
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();
    
    uint8_t DefaultFirstTeam = 0;

    if (current_playlist)
    {
        if (STATIC_HASPROPERTY(current_playlist, DefaultFirstTeam))
        {
            DefaultFirstTeam = current_playlist->DefaultFirstTeam();
        }
        else
        {
            DefaultFirstTeam = 2;
        }
    }
    else
    {
        DefaultFirstTeam = 2;
    }

    return DefaultFirstTeam;
}

int team_component::get_default_last_team()
{
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();
    
    uint8_t DefaultLastTeam = 0;

    if (current_playlist)
    {
        if (STATIC_HASPROPERTY(current_playlist, DefaultLastTeam))
        {
            DefaultLastTeam = current_playlist->DefaultLastTeam();
        }
        else
        {
            DefaultLastTeam = 102;
        }
    }
    else
    {
        DefaultLastTeam = 102;
    }

    return DefaultLastTeam;
}

uint8_t team_component::find_free_team(bool bMustBeEmpty, bool bMustBeReserved, std::wstring in_team_code)
{
    // List of all teams
    TArray<AFortTeamInfo*> all_teams = (*GWorld)->GameState()->Teams();
    
    const uint8_t DefaultFirstTeam = get_default_first_team();
    const uint8_t DefaultLastTeam = get_default_last_team();

    for (uint8_t TeamIdx = DefaultFirstTeam; TeamIdx <= DefaultLastTeam; TeamIdx++)
    {
        for (AFortTeamInfo* TeamInfo : all_teams)
        {
            if (TeamInfo->Team() == TeamIdx)
            {
                bool bIsValidReservedTeam = false;
                bool bIsReservedTeam = false;
                
                for (const auto& [team_code, reserved_team_idx] : reserved_teams)
                {
                    if (reserved_team_idx == TeamIdx && team_code == in_team_code)
                    {
                        bIsValidReservedTeam = true;
                        break;
                    }
                    if (reserved_team_idx == TeamIdx)
                    {
                        bIsReservedTeam = true;
                        break;
                    }
                }

                if (bIsValidReservedTeam == true && bMustBeReserved == true)
                {
                    if (TeamInfo->TeamMembers().Num() < get_team_size())
                    {
                        return TeamIdx;
                    }
                    
                    return 0;
                }
                
                if (bIsValidReservedTeam == false && bMustBeReserved == true)
                {
                    break;
                }

                if (bIsReservedTeam == true)
                {
                    break;
                }

                if (bMustBeEmpty == true)
                {
                    if (TeamInfo->TeamMembers().Num() == 0)
                    {
                        return TeamIdx;
                    }
                }
                else
                {
                    if (TeamInfo->TeamMembers().Num() < get_team_size())
                    {
                        return TeamIdx;
                    }
                }

                break;
            }
        }
    }

    return 0;
}

uint8_t team_component_hooks::PickTeam_Hook(AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController)
{
    // See if this PlayerController has a team code
    for (const auto& [team_code, players] : team_component::team_codes_map)
    {
        for (const AFortPlayerController* player : players)
        {
            // Found a team code with this playercontroller
            if (player == PlayerController)
            {
                if (const uint8_t found_team = team_component::find_free_team(false, true, team_code); found_team > 0)
                {
                    return found_team;
                }
                
                if (const uint8_t found_team = team_component::find_free_team(true, false); found_team > 0)
                {
                    if (team_component::reserved_teams.contains(team_code) == false)
                    {
                        team_component::reserved_teams[team_code] = found_team;
                    
                        return found_team;
                    }
                }
            }
        }
    }

    // else find a free team and make sure it's valid
    if (const uint8_t found_team = team_component::find_free_team(); found_team > 0)
    {
        return found_team;
    }

    spdlog::error(L"team_component_hooks::PickTeam_Hook implementation couldn't find a valid team, calling PickTeam...");
    
    return PickTeam(FortGameMode, InTeamIndex, PlayerController);
}