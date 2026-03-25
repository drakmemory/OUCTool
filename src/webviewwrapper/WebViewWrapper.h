#pragma once
#include <windows.h>
#include <string>
#include <wil/com.h>
#include <WebView2.h>

class WebViewWrapper {
public:
    bool Init(HWND hwnd, const std::wstring& url);
    // 根据指定 url 获取对应域名的 Cookie（同步等待回调返回，可能会阻塞）
    std::string GetCookieString(const std::wstring& url) const;

	std::string GetCurrentTitle() const;

    ICoreWebView2* Get();

private:
    void SetupEvents();
    wil::com_ptr<ICoreWebView2CookieManager> GetCookieManager() const;

private:
    HWND m_hwnd = nullptr;

    wil::com_ptr<ICoreWebView2> webview;
    wil::com_ptr<ICoreWebView2Controller> controller;

    std::wstring m_currentTitle;
};