#include "include.hpp"
#include "renderer.hpp"

#include <d3d11.h>
#include <dwmapi.h>
#include <tchar.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../resource.h"
#include "../components/config_manager.hpp"
#include "../components/loader_component.hpp"
#include "../components/elimination_component.hpp"
#include "../components/log_component.hpp"
#include "../components/net_component.hpp"
#include "../components/player_component.hpp"
#include "../components/shell_component.hpp"
#include "../components/director_client.hpp"
#include "../components/inventory_component.hpp"
#include "../utils/stb_image.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace ImGui {

    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col, ImVec2 position) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = position;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);

        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;

        const float a = speed*0;
        const float b = speed*0.333f;
        const float c = speed*0.666f;

        const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
        const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
        const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;

        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color,ImVec2 position) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = position;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        // Render
        window->DrawList->PathClear();

        int num_segments = 30;
        int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

        const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);

        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                                centre.y + ImSin(a+g.Time*8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
    }

}

renderer::renderer()
{
    bLoadedAssets = false;

    CreateNewWindow();
}

void renderer::CreateNewWindow()
{
    WNDCLASSEX WindowClass = { 0 }; 

    WindowClass.cbSize = sizeof WindowClass;
    WindowClass.style = CS_CLASSDC;
    WindowClass.lpfnWndProc = WndProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = GetModuleHandle(nullptr);
    WindowClass.hIcon = nullptr;
    WindowClass.hCursor = nullptr;
    WindowClass.hbrBackground = nullptr;
    WindowClass.lpszMenuName = nullptr;
    WindowClass.lpszClassName = TEXT("FortMP");
    WindowClass.hIconSm = nullptr;

    RegisterClassEx(&WindowClass);

    WindowHandle = CreateWindow(WindowClass.lpszClassName, TEXT("FortMP"), WS_CAPTION | WS_SYSMENU, 0, 0, 825, 595, nullptr, nullptr, WindowClass.hInstance, nullptr);

    if (CreateDeviceD3D(WindowHandle) == false)
    {
        spdlog::error(L"Failed to create Device D3D, leaving renderer creation...");
        
        return;
    }
    
    ShowWindow(WindowHandle, SW_SHOWDEFAULT);
    UpdateWindow(WindowHandle);

    HICON hIcon = static_cast<HICON>(::LoadImage(reinterpret_cast<HINSTANCE>(&__ImageBase),
    MAKEINTRESOURCE(IDI_ICON1),
    IMAGE_ICON,
    256, 256,
    LR_DEFAULTCOLOR));
    SendMessageA(WindowHandle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));

    // Dark mode
    BOOL UseDarkMode = true;
    DwmSetWindowAttribute(WindowHandle, 20, &UseDarkMode, sizeof UseDarkMode);
    
    ImGui::CreateContext();
    
    auto& IO = ImGui::GetIO();
    IO.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/seguisb.ttf", 17);
    auto& Style = ImGui::GetStyle();
    Style.TabRounding = 0.2f;
    Style.FrameRounding = 3.f;

    auto* Colors = Style.Colors;
    Colors[ImGuiCol_Text]                   = ImVec4(1.0f, 1.0f, 1.0f, 0.87f);
    Colors[ImGuiCol_WindowBg]               = ImVec4(0.058f, 0.05f, 0.07f, 0.94f);
    Colors[ImGuiCol_FrameBg]                = ImVec4(0.22f, 0.22f, 0.22f, 0.54f);
    Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.38f, 0.38f, 0.39f, 0.40f);
    Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.36f, 0.36f, 0.36f, 0.67f);
    Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    Colors[ImGuiCol_CheckMark]              = ImVec4(0.03f, 0.50f, 0.83f, 1.00f);
    Colors[ImGuiCol_SliderGrab]             = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    Colors[ImGuiCol_Button]                 = ImVec4(0.28f, 0.28f, 0.28f, 0.40f);
    Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
    Colors[ImGuiCol_ButtonActive]           = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
    Colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.31f);
    Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.17f, 0.17f, 0.18f, 0.80f);
    Colors[ImGuiCol_HeaderActive]           = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
    Colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.23f, 0.23f, 0.23f, 0.78f);
    Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    Colors[ImGuiCol_Tab]                    = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    Colors[ImGuiCol_TabHovered]             = ImVec4(0.163f, 0.160f, 0.20f, 0.80f);
    Colors[ImGuiCol_TabActive]              = ImVec4(0.163f, 0.160f, 0.20f, 1.00f);
    
    ImGui_ImplWin32_Init(WindowHandle);
    ImGui_ImplDX11_Init(pDevice, pDeviceContext);
    
    ImVec4 ClearColor = ImVec4(0.f, 0.f, 0.f, 1.00f);
    
    // Render
    bool bQuitWindow = false;

    while (bQuitWindow == false)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT)
            {
                bQuitWindow = true;
            }
        }
        
        if (bQuitWindow)
        {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Render
        ImGui::SetNextWindowSize(ImVec2(IO.DisplaySize.x, IO.DisplaySize.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        // If GWorld is not yet valid, don't render the tabs
        if (*GWorld == nullptr || (*GWorld)->NetDriver() == nullptr)
        {
            if (ImGui::Begin("LoadingWindow", reinterpret_cast<bool*>(true), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {
                ImGui::SetCursorPosX(IO.DisplaySize.x * .5 - ImGui::CalcTextSize("FortMP is loading").x * .5);
                ImGui::SetCursorPosY(IO.DisplaySize.y * .5 - ImGui::CalcTextSize("FortMP is loading").y * 2.25);
                
                ImGui::Text("FortMP is loading");

                float thickness = 4;
                float radius = 11;
                const ImU32 col = ImGui::GetColorU32(ImVec4(0.34f, 0.34f, 0.34f, 1.00f));
                ImVec2 spinner_pos = ImVec2(IO.DisplaySize.x * .5 - radius+thickness/2, IO.DisplaySize.y * .5);
                ImGui::Spinner("##spinner", radius, thickness, col, spinner_pos);
            }

            ImGui::End();
        }
        else
        {   
            if (ImGui::Begin("FortMPGUI", reinterpret_cast<bool*>(true), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {
                if (ImGui::BeginTabBar("MainTabBar"))
                {
                    if (ImGui::BeginTabItem("Log"))
                    {
                        if (ImGui::BeginChild("ConsoleContainer", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 31)))
                        {
                            for (auto& message : log_component::messages)
                            {
                                if (message.processed == false)
                                {
                                    message.processed = true;
                                
                                    ImGui::SetScrollY(ImGui::GetScrollMaxY());   
                                }
                            
                                ImGui::PushStyleColor(ImGuiCol_Text, message.color);
                                ImGui::Text(message.message.c_str());
                                ImGui::PopStyleColor();
                            }

                            ImGui::EndChild();
                        }

                        ImGui::Separator();
                        
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                        
                        if (ImGui::InputTextWithHint("", "Enter a console command...", strCommand, sizeof strCommand, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            std::string str_command(strCommand);

                            ZeroMemory(strCommand, sizeof strCommand);
                            
                            UKismetSystemLibrary::ExecuteConsoleCommand(std::wstring(str_command.begin(), str_command.end()).c_str());
                        }

                        ImGui::PopItemWidth();
                        
                        ImGui::EndTabItem();
                    }
                    
                    if (ImGui::BeginTabItem("World"))
                    {
                        if (ImGui::BeginChild("WorldDetails", ImVec2(IO.DisplaySize.x / 3.f, 0), true))
                        {
                            UFortPlaylistAthena** ppCurrentPlaylist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
                                &(*GWorld)->GameState()->CurrentPlaylistData() :
                                (*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo") ?
                                &(*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() :
                                nullptr;

                            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Match Info");

                            ImGui::Separator();
                            
                            ImGui::Text("Alive Players: %i/%i", (*GWorld)->GameState()->PlayersLeft(), (*GWorld)->AuthorityGameMode()->GameSession()->MaxPlayers());

                            ImGui::Text("Current Playlist: %s", ppCurrentPlaylist == nullptr ? "NULL" : *ppCurrentPlaylist ? (*ppCurrentPlaylist)->sGetName().c_str() : "NULL");
                            
                            static UEnum* EGamePhase = FindObject<UEnum>(L"/Script/FortniteGame.EAthenaGamePhase");
                            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                            ImGui::Text("Current Gamephase: %s", converter.to_bytes(EGamePhase->GetEnumName((*GWorld)->GameState()->GamePhase())).c_str());

                            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Match Shortcuts");

                            ImGui::Separator();
                            
                            if (ImGui::Button("Start Match", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 10.f;
                                (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 10.f;
                            }
                            
                            if (ImGui::Button("Restart Match", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                // To restart a match, kick everyone in the session
                                for (int ConnIdx = 0; ConnIdx < (*GWorld)->NetDriver()->ClientConnections().Num(); ConnIdx++)
                                {
                                    UNetConnection* CurrentConnection = (*GWorld)->NetDriver()->ClientConnections()[ConnIdx];

                                    if (CurrentConnection && CurrentConnection->PlayerController())
                                    {
                                        CurrentConnection->PlayerController()->ClientReturnToMainMenu(L"Restarting Server.");
                                    }
                                }

                                spdlog::info(L"Performing server restart.");
                            }

                            if (ImGui::Button("Safe Restart Server", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                loader_component::bPendingShutdown = true;
                            }

                            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Live Events");

                            ImGui::Separator();

                            if (ImGui::Button("Start Rocket Event", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> geode_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_GeodeScripting_C::StaticClass());
                                
                                for (auto actor : geode_scripters)
                                {
                                    static_cast<ABP_GeodeScripting_C*>(actor)->LaunchSequence();
                                }
                            }
                            
                            if (ImGui::Button("Experimental force connection loss", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                websocketpp::lib::error_code ec;
                                auto Connection = gameserver_client::Client->Client_WS.get_con_from_hdl(gameserver_client::Client->Connection_Handle, ec);
                                gameserver_client::Client->Client->Client_WS.close(Connection, 1000, "");
                            }

                            if (ImGui::Button("Start End Event", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> nightnight_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_NightNight_Scripting::StaticClass());
                                
                                for (auto actor : nightnight_scripters)
                                {
                                    static_cast<ABP_NightNight_Scripting*>(actor)->StartEvent();
                                }

                                for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
                                {
                                    UNetConnection* Connection = (*GWorld)->NetDriver()->ClientConnections()[i];

                                    if (Connection->PlayerController() == nullptr || Connection->PlayerController()->Pawn() == nullptr)
                                    {
                                        continue;
                                    }

                                    auto pickaxe = inventory_component::get_pickaxe(Connection->PlayerController())->Cast<UFortWorldItem>();

                                    if (pickaxe)
                                    {
                                        inventory_component::remove_item(Connection->PlayerController(), pickaxe->ItemEntry().ItemGuid(), 1, false, false);   
                                    }
                                }
                            }

                            if (ImGui::Button("Start Unvaulting Event", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> snow_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_SnowScripting::StaticClass());
                                
                                for (auto actor : snow_scripters)
                                {
                                    static_cast<ABP_SnowScripting*>(actor)->SnowStart();
                                }

                                for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
                                {
                                    UNetConnection* Connection = (*GWorld)->NetDriver()->ClientConnections()[i];

                                    if (Connection->PlayerController() == nullptr || Connection->PlayerController()->Pawn() == nullptr)
                                    {
                                        continue;
                                    }

                                    inventory_component::clear_primary_inventory(Connection->PlayerController(), false);
                                }
                            }


                            if (ImGui::Button("Start Project Era Festival", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> domino_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), Adomino_scripting_C::StaticClass());
                                
                                for (auto actor : domino_scripters)
                                {
                                    static_cast<Adomino_scripting_C*>(actor)->StartEvent();
                                }
                                
                                (*GWorld)->AuthorityGameMode()->bMatchHasStarted() = true;

#ifdef _RELEASE
                                gameserver_client::Client->GameServer_Status.accepting_players = false;
                                gameserver_client::Client->SendUpdate();

                                player_component::starting_players = gameserver_client::Client->GameServer_Status.number_of_players;

                                /*
                                 * Send a match_started event to director
                                 */
                                if (director_client::client)
                                {
                                    director_client::client->Send(eudp_packet({
                                        {"type", "match_started"},
                                        {"payload", {
                                            {"starting_players", player_component::starting_players}
                                        }}
                                    }));
                                }
#endif

                            }

                            if (ImGui::Button("Start NY Event", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> domino_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), FindObject(L"/Game/fortmp/content_simple/athena/maps/bp_fortmp_map_loader.bp_fortmp_map_loader_C", true, false));
                                
                                for (auto actor : domino_scripters)
                                {
                                    static_cast<Abp_fortmp_map_loader*>(actor)->StartNYE();
                                }
                            }
                            
                            if (ImGui::Button("Next Cube Step", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> cube_scripter = UGameplayStatics::GetAllActorsOfClass((*GWorld), ACUBE_C::StaticClass());
                                
                                for (auto actor : cube_scripter)
                                {
                                    static_cast<ACUBE_C*>(actor)->NextStepDynamic();
                                }
                            }

                            if (ImGui::Button("Ice King Event", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                game_thread_queue.push_back([]
                                {
                                    TArray<AActor*> mooney_scripting = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_MooneyScripting_C::StaticClass());
                                
                                    for (auto actor : mooney_scripting)
                                    {
                                
                                        static_cast<ABP_MooneyScripting_C*>(actor)->LoadSequenceLevel();
                                        spdlog::info(L"Called OnReady_9968C1F648044523426FE198948B0CC9");
                                        static_cast<ABP_MooneyScripting_C*>(actor)->OnReady_9968C1F648044523426FE198948B0CC9();
                                        spdlog::info(L"Called OnReady_9968C1F648044523426FE198948B0CC9");
                                        static_cast<ABP_MooneyScripting_C*>(actor)->BeginIceKingEvent();
                                        spdlog::info(L"Called BeginIceKingEvent");
                                    } 
                                });
                            }

                            if (ImGui::Button("Ice King Event PRE", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                TArray<AActor*> mooney_loader = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_MooneyLoader_C::StaticClass());
                                
                                for (auto actor : mooney_loader)
                                {
                                    static_cast<ABP_MooneyLoader_C*>(actor)->LoadMooneyMap() = true;
                                    static_cast<ABP_MooneyLoader_C*>(actor)->OnRep_LoadMooneyMap();
                                    spdlog::info(L"Called LoadMap");
                                }
                            }

                            if (ImGui::Button("Cube Drip", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                if(auto scripter = ABP_IslandScripting_C::get_island_scripting())
                                {
                                    if (scripter->CubeBP()->DrainAmount() >= 0.99f)
                                    {
                                        scripter->CubecrackLeadup();
                                        scripter->CubeBP()->DrainAmount() = 1;
                                        scripter->CubeBP()->BigDrip(true);
                                        spdlog::info(L"Last drip");

                                        scripter->ButterflyStart();
                                    }
                                    else
                                    {
                                        scripter->CubecrackLeadup();
                                        scripter->CubeBP()->BigDrip(false);
                                        scripter->CubeBP()->DrainAmount() = scripter->CubeBP()->DrainAmount() + 0.045f;
                                    
                                        spdlog::info(L"Drain amount: {}", scripter->CubeBP()->DrainAmount());   
                                    }
                                }
                            }
                            
                            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Miscellaneous");

                            ImGui::Separator();

                            if (ImGui::Button("Toggle Server Choose Part", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                auto config = config_manager::get_server_config();
                                bool scp_enabled = config["scp_enabled"];
                                config["scp_enabled"] = !scp_enabled;
                                config_manager::save_server_config(config);
                            }

                            if (ImGui::Button("Toggle Debug Logs", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                log_component::enable_debug_logs = !log_component::enable_debug_logs;
                            }

                            static bool keep_items_on_death = false;
                            
                            if (ImGui::Checkbox("Keep Inventory After Death", &keep_items_on_death))
                            {
                                elimination_component::keep_inventory_after_death = true;
                            }
                        
                            ImGui::EndChild();
                        }

                        ImGui::SameLine();
                        
                        ImVec2 WorldCursorPos = ImGui::GetCursorPos(); // 0 0 in our image
                        WorldCursorPos.x += 306;
                        WorldCursorPos.y += 313;
                        
                        ImGui::Image(pMapTexture, ImVec2(512, 512));

                        // Draw a blip for each NetConnection we have
                        for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
                        {
                            UNetConnection* Connection = (*GWorld)->NetDriver()->ClientConnections()[i];

                            if (Connection->PlayerController() == nullptr || Connection->PlayerController()->Pawn() == nullptr)
                            {
                                continue;
                            }
                            
                            FVector PawnLocation = Connection->PlayerController()->Pawn()->K2_GetActorLocation();
                            
                            // Get this pawn's location on the server map
                            ImVec2 PawnLocationRelative(PawnLocation.Y / 512 + WorldCursorPos.x, WorldCursorPos.y - PawnLocation.X / 512);

                            AFortPlayerControllerAthena* athena_controller = static_cast<AFortPlayerControllerAthena*>(Connection->PlayerController());
                            
                            // Draw the blip
                            ImGui::GetWindowDrawList()->AddCircle(PawnLocationRelative, 5, IM_COL32(0, 0, 0, 255));
                            ImGui::GetWindowDrawList()->AddCircleFilled(PawnLocationRelative, 4, IM_COL32(20, 90, 210, 255));

                            // If we are hovering the blip, render a tooltip
                            int CursorDistance = std::round(GetDistance(ImGui::GetMousePos(), PawnLocationRelative));
                            
                            if (CursorDistance <= 5) // If distance is less than radius, render tooltip
                            {
                                ImGui::BeginTooltip();

                                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
                                ImGui::Text(converter.to_bytes(Connection->PlayerController()->MyPlayerInfo()->PlayerName().c_str()).c_str());
                                
                                ImGui::EndTooltip();
                            }
                        }
                    
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Players"))
                    {
                        ImGui::BeginChild("PlayerListPanel", ImVec2(200, 0), true);

                        for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
                        {
                            UNetConnection* Connection = (*GWorld)->NetDriver()->ClientConnections()[i];

                            if (Connection == nullptr || Connection->PlayerController() == nullptr)
                            {
                                continue;
                            }

                            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
                            // Add player to the list
                            if (ImGui::Selectable(converter.to_bytes(Connection->PlayerController()->MyPlayerInfo()->PlayerName().c_str()).c_str(), i == selectedPlayerIndex))
                            {
                                selectedPlayerIndex = i;
                            }
                        }
                        
                        ImGui::EndChild();

                        ImGui::SameLine();

                        ImGui::BeginChild("PlayerInfoPanel", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);

                        if ((*GWorld)->NetDriver()->ClientConnections().Num() > selectedPlayerIndex)
                        {
                            UNetConnection* SelectedConnection = (*GWorld)->NetDriver()->ClientConnections()[selectedPlayerIndex];

                            if (SelectedConnection->PlayerController() && SelectedConnection->PlayerController()->Pawn())
                            {
                                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Player Info");
                                ImGui::Separator();
                        
                                ImGui::Text("Health: %i/100", (int)SelectedConnection->PlayerController()->Pawn()->GetHealth());
                                ImGui::Text("Shield: %i/100", (int)SelectedConnection->PlayerController()->Pawn()->GetShield());
                                
                                ImGui::Text("bIsInvulnerable: %s", SelectedConnection->PlayerController()->Pawn()->bIsInvulnerable() ? "true" : "false");

                                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Utilities");
                                ImGui::Separator();

                                if (ImGui::Button("Toggle God Mode"))
                                {
                                    UKismetSystemLibrary::SetBoolPropertyByName(SelectedConnection->PlayerController()->Pawn(), L"bIsInvulnerable", !SelectedConnection->PlayerController()->Pawn()->bIsInvulnerable());
                                }

                                if (ImGui::Button("Restart Player"))
                                {
                                    // Log state
                                    spdlog::warn(L"Manually restarting player. State: {}", SelectedConnection->PlayerController()->StateName().ToString().c_str());
                                    
                                    (*GWorld)->AuthorityGameMode()->RestartPlayer(SelectedConnection->PlayerController());
                                }
                                
                                if (ImGui::Button("Kill"))
                                {
                                    // Force suicide upon this player
                                    SelectedConnection->PlayerController()->ServerSuicide();
                                }

                                                                
                                if (ImGui::Button("Teleport all to player"))
                                {
                                    for (auto connection : (*GWorld)->NetDriver()->ClientConnections())
                                    {
                                        if (connection != SelectedConnection)
                                        {
                                            connection->PlayerController()->Pawn()->K2_TeleportTo(SelectedConnection->PlayerController()->Pawn()->K2_GetActorLocation() + FVector(0, 0, 500), FRotator());
                                        }
                                    }
                                }

                                if (ImGui::Button("Kick"))
                                {
                                    SelectedConnection->PlayerController()->ClientReturnToMainMenu(L"Kicked by a server operator.");
                                }
                            }
                        }
                        
                        ImGui::EndChild();

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Blueprint"))
                    {
                        static std::string SelectedScript;
                        
                        ImGui::BeginChild("BlueprintList", ImVec2(170, ImGui::GetContentRegionAvail().y - 31), true);
                        for (auto& [key, val] : BlueprintDisasm::DisassembledBlueprintFunctions)
                        {
                            if (key.size() > 0)
                            {
                                if (ImGui::Selectable(key.c_str(), SelectedScript == key))
                                {
                                    SelectedScript = key;
                                }
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();
                        
                        ImGui::BeginChild("LoggerContainer", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 31));
                        if (SelectedScript.size() > 0 && BlueprintDisasm::DisassembledBlueprintFunctions.contains(SelectedScript))
                        {
                            ImGui::TextUnformatted(BlueprintDisasm::DisassembledBlueprintFunctions[SelectedScript].dump(4).c_str());
                        }
                        ImGui::EndChild();

                        ImGui::Separator();
                        
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

                        static char BlueprintPath[255];
                        if (ImGui::InputTextWithHint("", "Enter a blueprint class path...", BlueprintPath, sizeof BlueprintPath, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            std::string str_command(BlueprintPath);
                            TO_WSTRING(str_command, wstr_command);

                            ZeroMemory(BlueprintPath, sizeof BlueprintPath);
                            
                            if (UObject* BlueprintClass = FindObject(wstr_command.c_str()))
                            {
                                for (UObject* Child = *reinterpret_cast<UObject**>(reinterpret_cast<__int64>(BlueprintClass) + CoreOffsets::Children); Child; Child = *reinterpret_cast<UObject**>(reinterpret_cast<__int64>(Child) + CoreOffsets::Next))
                                {
                                    if (Child->IsA(UFunction::StaticClass()))
                                    {
                                        if (!BlueprintDisasm::DisassembledBlueprintFunctions.contains(Child->sGetName().c_str()))
                                        {
                                            new BlueprintDisasm(static_cast<UFunction*>(Child));
                                        }
                                    }
                                }
                            }
                        }

                        ImGui::PopItemWidth();
                        
                        ImGui::EndTabItem();
                    }
                    
                    ImGui::EndTabBar();
                }
            }

            ImGui::End();
        }

        ImGui::Render();

        // Set clear color
        const float ClearColorAlpha[4] = { ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w };
        pDeviceContext->OMSetRenderTargets(1, &pRenderViewTarget, nullptr);
        pDeviceContext->ClearRenderTargetView(pRenderViewTarget, ClearColorAlpha);
        
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        static unsigned int PresentFlags = 0;
        
        if (pSwapChain->Present(1, PresentFlags) == DXGI_STATUS_OCCLUDED)
        {
            PresentFlags = DXGI_PRESENT_TEST;
            Sleep(1000 / 30);
        }
        else
        {
            PresentFlags = 0;
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyDeviceD3D();
    DestroyWindow(WindowHandle);
    UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);

    // Unregister server
#ifdef _RELEASE
    // Reset this server's info on Era Services
    gameserver_client::Client->SendLeave();
#endif
    
    if (*GWorld && (*GWorld)->NetDriver())
    {
        for (int ConnIdx = 0; ConnIdx < (*GWorld)->NetDriver()->ClientConnections().Num(); ConnIdx++)
        {
            UNetConnection* CurrentConnection = (*GWorld)->NetDriver()->ClientConnections()[ConnIdx];

            if (CurrentConnection && CurrentConnection->PlayerController())
            {
                CurrentConnection->PlayerController()->ClientReturnToMainMenu(L"Server Closed.");
            }
        }
    }
    
    spdlog::info(L"Closed GUI, terminating server...");
    
    // Close director connection
    if (director_client::client)
    {
        director_client::client->Close();
    }
    
    // Terminate program
    RequestExitWithStatus(true, 0);
}

bool renderer::CreateDeviceD3D(HWND WindowHandle)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof sd);
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = WindowHandle;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT CreateDeviceFlags = 0;
    D3D_FEATURE_LEVEL FeatureLevel;
    const D3D_FEATURE_LEVEL FeatureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, CreateDeviceFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &FeatureLevel, &pDeviceContext) != S_OK)
    {
        return false;
    }
    
    CreateRenderTarget();
    
    return true;
}

void renderer::DestroyDeviceD3D()
{
    DestroyRenderTarget();

    if (pSwapChain)
    {
        pSwapChain->Release();

        pSwapChain = nullptr;
    }
    if (pDeviceContext)
    {
        pDeviceContext->Release();

        pDeviceContext = nullptr;
    }
    if (pDevice)
    {
        pDevice->Release();

        pDevice = nullptr;
    }
}

void renderer::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;

    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderViewTarget);
    
    pBackBuffer->Release();
}

void renderer::DestroyRenderTarget()
{
    if (pRenderViewTarget)
    {
        pRenderViewTarget->Release();

        pRenderViewTarget = nullptr;
    }
}

bool renderer::CreateTextureFromTexture2D(UTexture2D* InTexture2D, ID3D11ShaderResourceView** OutShaderResourceView, int* OutWidth, int* OutHeight)
{
    if (InTexture2D == nullptr)
    {
        spdlog::error(L"renderer::CreateTextureFromTexture2D: Invalid InTexture2D");

        return false;
    }

    void* ImageData = nullptr;
    
    InTexture2D->PlatformData()->TryLoadMips(0, &ImageData);
    
    if (ImageData == nullptr)
    {
        spdlog::error(L"renderer::CreateTextureFromTexture2D: Invalid Texture ImageData");

        return false;
    }
    
    if (OutWidth)
    {
        *OutWidth = InTexture2D->PlatformData()->SizeX;
    }
    if (OutHeight)
    {
        *OutHeight = InTexture2D->PlatformData()->SizeY;
    }

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof desc);
    desc.Width = InTexture2D->PlatformData()->SizeX;
    desc.Height = InTexture2D->PlatformData()->SizeY;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_BC1_UNORM; 
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = ImageData;
    subResource.SysMemPitch = desc.Width * 2;
    subResource.SysMemSlicePitch = 0;

    pDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof srvDesc);
    srvDesc.Format = DXGI_FORMAT_BC1_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    pDevice->CreateShaderResourceView(pTexture, &srvDesc, OutShaderResourceView);
    pTexture->Release();

    FMemory_Free(ImageData);

    return true;   
}

void renderer::LoadAssets()
{
    if (bLoadedAssets == false) // Crashes on 1.8, better if disabled there
    {
        bLoadedAssets = true;

        UTexture2D* MapTexture = FindObject<UTexture2D>(L"/Game/Athena/Apollo/Maps/UI/Apollo_Terrain_Minimap.Apollo_Terrain_Minimap", true) ?
            FindObject<UTexture2D>(L"/Game/Athena/Apollo/Maps/UI/Apollo_Terrain_Minimap.Apollo_Terrain_Minimap", true) :
            FindObject<UTexture2D>(L"/Game/Athena/HUD/MiniMap/MiniMapAthena.MiniMapAthena", true);
        CreateTextureFromTexture2D(MapTexture, &pMapTexture, nullptr, nullptr);
    }
}

float renderer::GetDistance(ImVec2 A, ImVec2 B)
{
    const auto DistanceX = A.x - B.x;
    const auto DistanceY = A.y - B.y;

    return std::sqrt(DistanceX * DistanceX + DistanceY * DistanceY);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }
    
    switch (msg)
    {
    case WM_SIZE:
        {
            if (pDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                DestroyRenderTarget();
                pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            
            return 0;
        }
    case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU)
            {
                return 0;
            }

            break;
        }
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}