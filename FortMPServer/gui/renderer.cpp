#include "include.hpp"
#include "renderer.hpp"

#include <d3d11.h>
#include <dwmapi.h>
#include <tchar.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

renderer::renderer()
{
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

    HWND WindowHandle = CreateWindow(WindowClass.lpszClassName, TEXT("FortMP"), WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, nullptr, nullptr, WindowClass.hInstance, nullptr);

    if (CreateDeviceD3D(WindowHandle) == false)
    {
        spdlog::error(L"Failed to create Device D3D, leaving renderer creation...");
        
        return;
    }
    
    ShowWindow(WindowHandle, SW_SHOWDEFAULT);
    UpdateWindow(WindowHandle);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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

    spdlog::info(L"Closed GUI, terminating server...");
    
    //TODO: RequestExit
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