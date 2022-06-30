#include "include.hpp"
#include "player_tab.hpp"

#include "imgui/imgui.h"

void player_tab::render()
{
    // All connected clients
    TArray<UNetConnection*>& ClientConnections = (*GWorld)->NetDriver()->ClientConnections();

    if (ImGui::Begin("Players", nullptr, ImGuiWindowFlags_None))
    {
        ImGui::BeginChild("player_list_left_pane", ImVec2(200, 0), true);

        for (int connection_idx = 0; connection_idx < ClientConnections.Num(); connection_idx++)
        {
            UNetConnection* client_connection = ClientConnections[connection_idx];

            if (client_connection == nullptr)
            {
                continue;
            }

            // TODO: Get real player name
            const char* player_name = "test";
            
            if (ImGui::Selectable(player_name, connection_idx == selected_player_idx))
            {
                // Update player index
                selected_player_idx = connection_idx;
            }
        }

        ImGui::EndChild();
    }

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

void player_tab::render_player_list()
{
    // All connected clients
    TArray<UNetConnection*>& ClientConnections = (*GWorld)->NetDriver()->ClientConnections();

    if (ClientConnections.Num() > 0)
    {
        UNetConnection* net_connection = (*GWorld)->NetDriver()->ClientConnections()[selected_player_idx];

        if (net_connection->PlayerController())
        {
            // Render player information
            render_player(net_connection->PlayerController());
        }
    }
}

void player_tab::render_player(AFortPlayerController* player_controller)
{
    ImGui::BeginGroup();
    ImGui::BeginChild("player_info_view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    
    if (player_controller == nullptr)
    {
        return;
    }

    // TODO: Get player name for controller
    ImGui::Text("player idx: %d", selected_player_idx);
    ImGui::Separator();

    ImGui::Checkbox("bHasServerFinishedLoading", &player_controller->bHasServerFinishedLoading());

    if (player_controller->Pawn())
    {
        // Get location and rotation
        FVector actor_location = player_controller->Pawn()->K2_GetActorLocation();
        FRotator actor_rotation = player_controller->Pawn()->K2_GetActorRotation();
        
        ImGui::Text("location = x=%f, y=%f, z=%f",
            actor_location.X, actor_location.Y, actor_location.Z);

        ImGui::Text("rotation = pitch=%f, yaw=%f, roll=%f",
            actor_rotation.Pitch, actor_rotation.Yaw, actor_rotation.Roll);
    }
}
