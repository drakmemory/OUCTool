#pragma once
#include <Windows.h>

class Window
{
public:
    Window(const char* title, int width, int height);
    ~Window();

    bool ShouldClose() const;
    void PollEvents();
    HWND GetHWND() const { return hwnd; }
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
    HWND hwnd = nullptr;
    bool quitMessageReceived = false;
    int width = 0;
    int height = 0;

    void ApplyCornerStyle();
    bool IsWin11OrGreater() const;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
