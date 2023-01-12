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
#include "../components/loader_component.hpp"
#include "../components/log_component.hpp"
#include "../components/net_component.hpp"
#include "../components/player_component.hpp"
#include "../utils/stb_image.h"

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

    HWND WindowHandle = CreateWindow(WindowClass.lpszClassName, TEXT("FortMP"), WS_CAPTION | WS_SYSMENU, 0, 0, 825, 595, nullptr, nullptr, WindowClass.hInstance, nullptr);

    if (CreateDeviceD3D(WindowHandle) == false)
    {
        spdlog::error(L"Failed to create Device D3D, leaving renderer creation...");
        
        return;
    }
    
    ShowWindow(WindowHandle, SW_SHOWDEFAULT);
    UpdateWindow(WindowHandle);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    auto& IO = ImGui::GetIO();
    IO.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 17);

    auto& Style = ImGui::GetStyle();
    Style.TabRounding = 0.2f;
    Style.FrameRounding = 3.f;

    auto* Colors = Style.Colors;
    Colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);
    Colors[ImGuiCol_FrameBg]                = ImVec4(0.22f, 0.22f, 0.22f, 0.54f);
    Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.38f, 0.38f, 0.39f, 0.40f);
    Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.36f, 0.36f, 0.36f, 0.67f);
    Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    Colors[ImGuiCol_CheckMark]              = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
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
    Colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.86f);
    Colors[ImGuiCol_TabHovered]             = ImVec4(0.35f, 0.35f, 0.35f, 0.80f);
    Colors[ImGuiCol_TabActive]              = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    
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
                ImGui::SetCursorPosX(IO.DisplaySize.x * .5 - ImGui::CalcTextSize("FortMP is loading...").x * .5);
                ImGui::SetCursorPosY(IO.DisplaySize.y * .5 - ImGui::CalcTextSize("FortMP is loading...").y);
                
                ImGui::Text("FortMP is loading...");
            }

            ImGui::End();
        }
        else
        {   
            if (ImGui::Begin("FortMPGUI", reinterpret_cast<bool*>(true), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
            {
                if (ImGui::BeginTabBar("MainTabBar"))
                {
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

                            ImGui::Text("Alive Players: %i/%i", (*GWorld)->GameState()->PlayersLeft(), (*GWorld)->NetDriver()->ClientConnections().Num());

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
                                    else if (CurrentConnection)
                                    {
                                        NetConnection__Close(CurrentConnection);
                                    }
                                }

                                spdlog::info(L"Performing server restart.");
                            }

                            if (ImGui::Button("Safe Restart Server", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                loader_component::bPendingShutdown = true;
                            }

                            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "Miscellaneous");

                            ImGui::Separator();

                            ImGui::Checkbox("Enable Server Choose Part", &player_component::bEnableServerChoosePart);
                        
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

                            // Draw the blip
                            ImGui::GetWindowDrawList()->AddCircle(PawnLocationRelative, 5, IM_COL32(0, 0, 0, 255));
                            ImGui::GetWindowDrawList()->AddCircleFilled(PawnLocationRelative, 4, IM_COL32(255, 255, 255, 255));

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

                                if (ImGui::Button("Kill"))
                                {
                                    // Force suicide upon this player
                                    SelectedConnection->PlayerController()->ServerSuicide();
                                }

                                if (ImGui::Button("Kick"))
                                {
                                    SelectedConnection->PlayerController()->ClientReturnToMainMenu(L"Kicked by a server operator.");
                                    NetConnection__Close(SelectedConnection);
                                }
                            }
                        }
                        
                        ImGui::EndChild();

                        ImGui::EndTabItem();
                    }

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
        
        pSwapChain->Present(1, 0);
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
            else if (CurrentConnection)
            {
                NetConnection__Close(CurrentConnection);
            }
        }
    }
    
    spdlog::info(L"Closed GUI, terminating server...");
    
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