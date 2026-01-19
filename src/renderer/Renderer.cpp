#include "Renderer.h"
#include <d3d11.h>
#include <dxgi.h>
#include <stdexcept>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

Renderer::Renderer(HWND hwnd_, int width, int height) : hwnd(hwnd_)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scd,
        &swapChain,
        &device,
        nullptr,
        &context
    );

    if (FAILED(hr))
        throw std::runtime_error("创建HttpClient1设备和交换链失败");

    // 创建 RenderTargetView
    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr) || !backBuffer)
        throw std::runtime_error("无法取回缓冲区");
    hr = device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();
    if (FAILED(hr))
        throw std::runtime_error("创建RenderTargetView失败");
    context->OMSetRenderTargets(1, &rtv, nullptr);
    D3D11_VIEWPORT vp;
    vp.Width = (float)width; // 需要从构造函数传入宽和高
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);
}

Renderer::~Renderer()
{
    if (rtv) rtv->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
}

void Renderer::BeginFrame()
{
    context->OMSetRenderTargets(1, &rtv, nullptr);
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(rtv, clearColor);
}

void Renderer::EndFrame()
{
    swapChain->Present(1, 0);
}
