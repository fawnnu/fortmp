#include "include.hpp"
#include "world_tab.hpp"

#include "imgui/imgui.h"

void world_tab::render()
{
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 80);
                    
    if (ImGui::InputTextWithHint("", "Enter a console command", command, sizeof command, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        execute_command();
    }

    ImGui::PopItemWidth();

    ImGui::SameLine();
                    
    if (ImGui::Button("Execute"))
    {
        execute_command();
    }
                    
    if (ImGui::Button("Start Game"))
    {
        // Reduce warmup time
        (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 10.0f;
        (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 10.0f;
    }

    if (ImGui::Button("Set Christmas Bus"))
    {
        // Enable christmas bus
        if (UObject* BattleBus = FindObject(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus", true))
        {
            (*GWorld)->GameState()->DefaultBattleBus() = BattleBus;
        }
    }

    ImGui::Separator();

    // Display number of alive players
    ImGui::Text("Alive players: %d", (*GWorld)->AuthorityGameMode()->AlivePlayers().Num());
}

void world_tab::execute_command()
{
    // execute console command
    std::string str_command(command);
    
    UKismetSystemLibrary::ExecuteConsoleCommand(std::wstring(str_command.begin(), str_command.end()).c_str());
    
    memset(command, 0, sizeof command); // resets the command string   
}