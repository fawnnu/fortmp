#include "include.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include "imgui/imgui.h"

#include "../components/loader_component.hpp"

#include "tabs/player_tab.hpp"

void window::render(GLFWwindow* in_window)
{
    int width;
    int height;

    // Get window width and height
    glfwGetWindowSize(in_window, &width, &height);
    
    // Get ImGui to fill window
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    
    if (loader_component::is_server_ready)
    {
        {
            if (ImGui::Begin("World", nullptr, ImGuiWindowFlags_None))
            {
                if (ImGui::Button("Start Game"))
                {
                    // Reduce warmup time
                    (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 10.0f;
                    (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 10.0f;
                }
            }

            ImGui::End();
        }
        
        // Render player tab
        player_tab::render();
    }
}
