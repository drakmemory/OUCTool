#pragma once
#include <d3d11.h>

class Renderer
{
public:
    Renderer(HWND hwnd, int width, int height);
    ~Renderer();

    void BeginFrame();
    void EndFrame();

    ID3D11Device* GetDevice() const { return device; }
    ID3D11DeviceContext* GetContext() const { return context; }
    ID3D11RenderTargetView* GetRTV() {
		return rtv;
    }

private:
    HWND hwnd = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
};
