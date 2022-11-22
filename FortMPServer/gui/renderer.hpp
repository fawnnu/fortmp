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

    static bool CreateTextureFromTexture2D(UTexture2D* InTexture2D, class ID3D11ShaderResourceView** OutShaderResourceView, int* OutWidth, int* OutHeight);
    static void LoadAssets();

    static float GetDistance(struct ImVec2 A, ImVec2 B);
    
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    
    static inline class IDXGISwapChain* pSwapChain;
    static inline class ID3D11Device* pDevice;
    static inline class ID3D11DeviceContext* pDeviceContext;
    static inline class ID3D11RenderTargetView* pRenderViewTarget;

    static inline ID3D11ShaderResourceView* pMapTexture;
    static inline char strCommand[255];

    static inline int selectedPlayerIndex;
    
public:

    static inline bool bLoadedAssets;
};
