#include "ImGuiLayer.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

ImGuiLayer::ImGuiLayer(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // --- 加载中文 ---
    // 加载微软雅黑（Windows 自带路径）
    // GetGlyphRangesChineseFull() 包含简体和繁体
    font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    fontSmall = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 12.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
	fontBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    fontLarge = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 24.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    // ----------------
	io.IniFilename = nullptr; // 禁用 imgui.ini 文件
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 0.f;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}

void ImGuiLayer::Begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
