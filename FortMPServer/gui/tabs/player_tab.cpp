#include "include.hpp"
#include "player_tab.hpp"

#include "../../components/inventory_component.hpp"
#include "../../components/looting_component.hpp"
#include "imgui/imgui.h"

void player_tab::render()
{
    // All connected clients
    TArray<UNetConnection*>& ClientConnections = (*GWorld)->NetDriver()->ClientConnections();

    ImGui::BeginChild("player_list_left_pane", ImVec2(200, 0), true);

    for (int connection_idx = 0; connection_idx < ClientConnections.Num(); connection_idx++)
    {
        UNetConnection* client_connection = ClientConnections[connection_idx];

        if (client_connection == nullptr || client_connection->PlayerController() == nullptr ||client_connection->PlayerController()->MyPlayerInfo() == nullptr)
        {
            continue;
        }

        const wchar_t* player_name_wide = client_connection->PlayerController()->MyPlayerInfo()->PlayerName().c_str();
        
        char player_name[255];

        WideCharToMultiByte(CP_UTF8, 0, player_name_wide, -1, player_name, sizeof player_name, nullptr, nullptr);
        
        if (ImGui::Selectable(player_name, connection_idx == selected_player_idx))
        {
            // Update player index
            selected_player_idx = connection_idx;
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();
    
    if (ClientConnections.Num() > 0)
    {
        UNetConnection* net_connection = (*GWorld)->NetDriver()->ClientConnections()[selected_player_idx];
    
        if (net_connection->PlayerController())
        {
            // Render player information
            render_player(net_connection->PlayerController());
        }
    }

    ImGui::End();
}

void player_tab::render_player(AFortPlayerController* player_controller)
{
    ImGui::BeginGroup();
    ImGui::BeginChild("player_info_view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    if (ImGui::Button("God"))
    {
        // Make pawn invulnerable
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bIsInvulnerable", !player_controller->Pawn()->bIsInvulnerable());
    }
    
    if (ImGui::Button("Kick"))
    {
        player_controller->ClientReturnToMainMenu(L"Kicked by an operator.");
    }

    if (ImGui::Button("Ban"))
    {
        auto ban_list = dll_utils::get_banlist();

        char player_ip[255];
        WideCharToMultiByte(CP_UTF8, 0, player_controller->PlayerState()->SavedNetworkAddress().c_str(), -1, player_ip, sizeof player_ip, nullptr, nullptr);

        ban_list["Bans"].push_back(std::string(player_ip));

        std::ofstream ban_file;

        ban_file.open(std::string(dll_utils::get_current_path()) + "/FortMP/Bans.json");

        ban_file << std::setw(4) << ban_list << std::endl;

        ban_file.close();
        
        player_controller->ClientReturnToMainMenu(L"Banned by an operator.");
    }
}