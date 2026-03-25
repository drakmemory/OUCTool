include(FetchContent)

# 定义版本号，方便后续升级
set(WEBVIEW2_VERSION "1.0.3856.49")

FetchContent_Declare(
    webview2_sdk
    URL "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/${WEBVIEW2_VERSION}"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

message(STATUS "Fetching WebView2 SDK ${WEBVIEW2_VERSION}...")
FetchContent_MakeAvailable(webview2_sdk)

# 自动根据系统架构（x64 或 x86）设置库路径
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WV2_ARCH "x64")
else()
    set(WV2_ARCH "x86")
endif()

# 设置便捷变量供主工程使用
set(WEBVIEW2_STATIC_LIB "${webview2_sdk_SOURCE_DIR}/build/native/${WV2_ARCH}/WebView2LoaderStatic.lib")
set(WEBVIEW2_INCLUDE_DIR "${webview2_sdk_SOURCE_DIR}/build/native/include")
set(WEBVIEW2_LIBRARY "${webview2_sdk_SOURCE_DIR}/build/native/${WV2_ARCH}/WebView2Loader.dll.lib")
set(WEBVIEW2_LOADER_DLL "${webview2_sdk_SOURCE_DIR}/build/native/${WV2_ARCH}/WebView2Loader.dll")

message(STATUS "WebView2 SDK fetched to: ${webview2_sdk_SOURCE_DIR}")