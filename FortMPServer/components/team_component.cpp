#include "include.hpp"
#include "team_component.hpp"

#include "loader_component.hpp"

team_component::team_component()
{
    max_team_size_override = 1;
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%d], actual: [%d]"), team_component_hooks::PickTeam_Hook);
    REGISTER_FUNCTION_DETOUR(team_component_hooks::PickTeam, memory_manager::find_function_by_string(L"PickTeam for [%s], preferred: [%s], actual: [%s]"), team_component_hooks::PickTeam_Hook);
        
    DetourTransactionCommit();
}

void team_component::InitializeTeams()
{
    // Loop through every available team
    for (uint8_t TeamIdx = get_default_first_team(); TeamIdx <= get_default_last_team(); TeamIdx++)
    {
        for (AFortTeamInfo* TeamInfo : (*GWorld)->GameState()->Teams())
        {
            if (TeamInfo->Team() == TeamIdx)
            {
                new era_team(TeamInfo);
                
                break;
            }
        }
    }
}

void team_component::Login(AFortPlayerController* player_controller, const FString& Options)
{
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString EncryptionToken = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (EncryptionToken.IsValid())
        {
            std::wstring wEncryptionToken = EncryptionToken.c_str();

            for (auto& [team_id, team_info] : TeamsMap)
            {
                for (std::wstring& TeamMemberEncryptionToken : team_info.TeamMembers)
                {
                    if (TeamMemberEncryptionToken == wEncryptionToken)
                    {
                        // Add the player controller to this team
                        team_info.TeamMembersControllers.push_back(player_controller);

                        return;
                    }
                }
            }
        }
    }
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

era_team* team_component::find_era_team(AFortPlayerController* player_controller)
{
    for (era_team* team : TeamsList)
    {
        if (AFortTeamInfo* team_info = team->bound_team_info)
        {
            for (AController* team_member : team_info->TeamMembers())
            {
                if (team_member == player_controller)
                {
                    // Return found team
                    return team;
                }
            }
        }
    }
    
    return nullptr;   
}

int team_component::get_team_size()
{
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

int team_component::get_alive_teams()
{
    // Alive teams list
    std::vector<AFortTeamInfo*> alive_teams;

    // List of all teams
    TArray<AFortTeamInfo*> all_teams = (*GWorld)->GameState()->Teams();

    for (AFortTeamInfo* team_info : all_teams)
    {
        for (AController* team_member : team_info->TeamMembers())
        {
            if ((*GWorld)->AuthorityGameMode()->AlivePlayers().Find(static_cast<AFortPlayerController*>(team_member)))
            {
                alive_teams.push_back(team_info);
                break;
            }
        }
    }

    return alive_teams.size();
}

era_team* team_component::find_free_team(AFortPlayerController* player_controller)
{
    bool found_team = false;
    
    int number_of_teammates = 0;
    era_team_info* team_info = nullptr;

    // Gather information about the current player's team
    for (auto& [current_team_id, team_information] : TeamsMap)
    {
        if (found_team)
        {
            break;
        }
        
        for (AFortPlayerController* TeamMember : team_information.TeamMembersControllers)
        {
            // If the current team has our player as member, get all the needed information
            if (TeamMember == player_controller)
            {
                number_of_teammates = team_information.TeamMembers.size();
                team_info = &team_information;
                found_team = true;
                break;
            }
        }
    }

    if (found_team)
    {
        // Check if there's an appropriate team available for filling
        if (team_info->fill)
        {
            // Make sure preferred team is invalid
            if (team_info->preferred_team < get_default_first_team() || team_info->preferred_team > get_default_last_team())
            {
                for (era_team* team_instance : TeamsList)
                {
                    if (team_instance->status == era_team_status::available_for_fill)
                    {
                        if (team_instance->get_team_members() + number_of_teammates <= get_team_size())
                        {
                            team_info->preferred_team = team_instance->get_team_index();
                            team_instance->get_team_members() += number_of_teammates;
                        }
                    }
                }
            }
        }
    
        if (team_info->preferred_team >= get_default_first_team() && team_info->preferred_team <= get_default_last_team())
        {
            for (era_team* team_instance : TeamsList)
            {
                if (team_instance->status == era_team_status::available_for_fill || team_instance->status == era_team_status::available_for_teammates)
                {
                    if (team_instance->get_team_index() == team_info->preferred_team && team_instance->get_team_members() <= get_team_size())
                    {
                        return team_instance;
                    }
                }
            }
        }

        // Find an empty team if no suitable option had been found
        for (era_team* team_instance : TeamsList)
        {
            if (team_instance->status == era_team_status::empty)
            {
                // Claim the team for teammates
                team_instance->status = team_info->fill ? era_team_status::available_for_fill : era_team_status::available_for_teammates;

                // Make all teammates join this team
                team_info->preferred_team = team_instance->get_team_index();

                team_instance->get_team_members() += number_of_teammates;
                return team_instance;
            }
        }
    }
    
    return nullptr;   
}

uint8_t team_component_hooks::PickTeam_Hook(AFortGameMode* FortGameMode, uint8_t InTeamIndex, AFortPlayerController* PlayerController)
{
    if (era_team* picked_team = team_component::find_free_team(PlayerController))
    {
        return picked_team->get_team_index();
    }

    spdlog::error(L"team_component_hooks::PickTeam_Hook implementation couldn't find a valid team, calling PickTeam...");
    
    return PickTeam(FortGameMode, InTeamIndex, PlayerController);
}