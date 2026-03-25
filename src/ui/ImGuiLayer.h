#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "imgui.h"
#include <webviewwrapper/WebViewWrapper.h>

class ImGuiLayer
{
public:
    ImGuiLayer(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

    void Begin();
    void End();
    WebViewWrapper WebView;
    HWND webHWND;
    ImFont* font;
    ImFont* fontSmall;
    ImFont* fontBold;
    ImFont* fontLarge;
};
