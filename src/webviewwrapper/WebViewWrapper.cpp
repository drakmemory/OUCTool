#include "WebViewWrapper.h"

#include <wrl.h>
#include <wrl/event.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>

// 使用工具模块中的通用转换函数
#include "../utils.hpp"

using namespace Microsoft::WRL;

namespace {
std::wstring ToOriginUrl(const std::wstring& url) {
    const auto schemePos = url.find(L"://");
    if (schemePos == std::wstring::npos) return url;

    const auto slashPos = url.find(L'/', schemePos + 3);
    if (slashPos == std::wstring::npos) return url;

    return url.substr(0, slashPos);
}

std::string ReadCookieHeader(ICoreWebView2CookieList* list) {
    if (!list) return {};

    UINT count = 0;
    list->get_Count(&count);

    std::string cookies;
    for (UINT i = 0; i < count; i++) {
        wil::com_ptr<ICoreWebView2Cookie> cookie;
        list->GetValueAtIndex(i, cookie.put());

        wil::unique_cotaskmem_string name;
        wil::unique_cotaskmem_string value;
        cookie->get_Name(name.put());
        cookie->get_Value(value.put());

        if (name && value) {
            cookies += Utf16ToUtf8(name.get());
            cookies += "=";
            cookies += Utf16ToUtf8(value.get());
            cookies += "; ";
        }
    }

    return cookies;
}
}

bool WebViewWrapper::Init(HWND hwnd, const std::wstring& url) {
    m_hwnd = hwnd;

    return SUCCEEDED(
        CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this, url](HRESULT, ICoreWebView2Environment* env) -> HRESULT {

                    env->CreateCoreWebView2Controller(
                        m_hwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this, url](HRESULT, ICoreWebView2Controller* ctrl) -> HRESULT {
                                this->controller = ctrl;
                                this->controller->get_CoreWebView2(this->webview.put());
                                ShowWindow(m_hwnd, SW_HIDE);
                                RECT bounds;
                                GetClientRect(m_hwnd, &bounds);
                                this->controller->put_Bounds(bounds);
                                webview->Navigate(url.c_str());
                                SetupEvents();

                                return S_OK;
                            }).Get());

                    return S_OK;
                }).Get()
                    )
    );
}

void WebViewWrapper::SetupEvents() {
    if (!webview) return;

    webview->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT {
			wil::unique_cotaskmem_string title;
                webview->get_DocumentTitle(title.put());
                if (title)
                    m_currentTitle = title.get();
                else
                    m_currentTitle.clear();

                return S_OK;
            }).Get(),
                nullptr
                );
}

wil::com_ptr<ICoreWebView2CookieManager> WebViewWrapper::GetCookieManager() const {
    if (!webview) return {};

    wil::com_ptr<ICoreWebView2_2> webview2;
    webview->QueryInterface(IID_PPV_ARGS(&webview2));
    if (!webview2) return {};

    wil::com_ptr<ICoreWebView2CookieManager> manager;
    webview2->get_CookieManager(manager.put());
    return manager;
}

std::string WebViewWrapper::GetCookieString(const std::wstring& url) const {
    const auto manager = GetCookieManager();
    if (!manager) return {};

    const std::wstring primary = ToOriginUrl(url);
    const std::wstring secondary = (primary == url) ? std::wstring() : url;

    auto fetchCookies = [&manager](const std::wstring& queryUrl) -> std::string {
        if (queryUrl.empty()) return {};

        std::string result;
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        manager->GetCookies(
            queryUrl.c_str(),
            Callback<ICoreWebView2GetCookiesCompletedHandler>(
                [&result, &mtx, &cv, &done](HRESULT, ICoreWebView2CookieList* list) -> HRESULT {
                    const std::string tmp = ReadCookieHeader(list);

                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        result = tmp;
                        done = true;
                    }
                    cv.notify_one();
                    return S_OK;
                }).Get()
        );

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (done) break;
            }

            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!done) {
                return {};
            }
        }

        return result;
    };

    std::string cookies = fetchCookies(primary);
    if (!cookies.empty()) return cookies;

    if (!secondary.empty()) {
        cookies = fetchCookies(secondary);
        if (!cookies.empty()) return cookies;
    }

    return {};
}

ICoreWebView2* WebViewWrapper::Get() {
    return webview.get();
}

std::string WebViewWrapper::GetCurrentTitle() const
{
    return Utf16ToUtf8(m_currentTitle);
}