#pragma once

class renderer
{
public:
    renderer();

    static void CreateNewWindow();
    static bool CreateDeviceD3D(HWND WindowHandle);
    static void DestroyDeviceD3D();
    
    static void CreateRenderTarget();
    static void DestroyRenderTarget();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    
    static inline class IDXGISwapChain* pSwapChain;
    static inline class ID3D11Device* pDevice;
    static inline class ID3D11DeviceContext* pDeviceContext;
    static inline class ID3D11RenderTargetView* pRenderViewTarget;
};
