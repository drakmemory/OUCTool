#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "imgui.h"

class ImGuiLayer
{
public:
    ImGuiLayer(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

    void Begin();
    void End();
    ImFont* font;
    ImFont* fontSmall;
    ImFont* fontBold;
    ImFont* fontLarge;
};
