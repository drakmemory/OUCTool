#include "Window.h"
#include <VersionHelpers.h>
#include <dwmapi.h>
#include <imgui_internal.h>
#pragma comment(lib, "dwmapi.lib")

// 声明外部 ImGui 消息处理器
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef NTSTATUS(WINAPI* PFN_RtlGetVersion)(PRTL_OSVERSIONINFOW);

Window::Window(const char* title, int w, int h)
    : width(w), height(h)
{
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window::WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "OUCToolWindowClass";
    // 尝试从资源加载程序图标（资源脚本中使用 id 1）并设置到窗口类
    HICON hIconBig = (HICON)LoadImageA(wc.hInstance, MAKEINTRESOURCEA(1), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImageA(wc.hInstance, MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (hIconBig) wc.hIcon = hIconBig;
    if (hIconSmall) wc.hIconSm = hIconSmall;
    RegisterClassExA(&wc);

    // 居中
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        title,
        WS_POPUP | WS_VISIBLE,
        x, y, width, height,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );
    // 如果窗口成功创建，再次设置窗口图标（确保任务栏与 Alt-Tab 显示）
    if (hwnd) {
        if (hIconBig) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    ApplyCornerStyle();
}

Window::~Window()
{
    if (hwnd) DestroyWindow(hwnd);
}

bool Window::ShouldClose() const
{
    return quitMessageReceived;
}

void Window::PollEvents()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            quitMessageReceived = true;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// --------------------- 系统圆角 / 自绘圆角 ---------------------
void Window::ApplyCornerStyle()
{
    if (IsWin11OrGreater())
    {
        // Win11 系统圆角
        DWM_WINDOW_CORNER_PREFERENCE pref = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
            &pref, sizeof(pref));
    }
    else
    {
        // Win10 自绘圆角
        int radius = 16;
        HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, radius, radius);
        SetWindowRgn(hwnd, hRgn, TRUE);
    }
}

bool Window::IsWin11OrGreater() const
{
    HMODULE hMod = GetModuleHandleA("ntdll.dll");
    if (hMod)
    {
        PFN_RtlGetVersion pRtlGetVersion = (PFN_RtlGetVersion)GetProcAddress(hMod, "RtlGetVersion");
        if (pRtlGetVersion)
        {
            RTL_OSVERSIONINFOW osvi = { 0 };
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (pRtlGetVersion(&osvi) == 0) // STATUS_SUCCESS
            {
                // Win11 的核心版本虽然还是 10.0，但 BuildNumber 必须 >= 22000
                return (osvi.dwMajorVersion >= 10 && osvi.dwBuildNumber >= 22000);
            }
        }
    }
    return false; // 回退默认值
}

// --------------------- 可拖动 ---------------------
LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 转发给 ImGui 处理
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCHITTEST:
    {
        LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
        if (hit == HTCLIENT)
        {
            ImGuiContext* g = ImGui::GetCurrentContext();
            if (g && g->HoveredWindow != nullptr)
            {
                // 如果鼠标悬停在任何 ImGui 窗口上，返回 HTCLIENT 确保控件可点击
                if (g->HoveredId != 0 || g->ActiveId != 0)
                    return HTCLIENT;
            }
            return HTCAPTION; // 点击客户区也可拖动
        }
        return hit;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
