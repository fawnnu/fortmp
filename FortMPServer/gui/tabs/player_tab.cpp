#include "include.hpp"
#include "player_tab.hpp"

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

    if (ImGui::Button("Kick"))
    {
        player_controller->ClientReturnToMainMenu(L"Kicked by an operator.");
    }
}
