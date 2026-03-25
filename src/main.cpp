#include "app/App.h"

int main()
{
    const wchar_t* wt = L"此程序由暗忆制作免费开源，如果你是购买的，那恭喜你被骗了";
    MessageBoxW(NULL, wt, L"国开刷课器", MB_OK);

    std::wstring wstr(wt);
    std::string utf8 = Utf16ToUtf8(wstr);
    // 确保控制台使用 UTF-8 编码输出，再打印 UTF-8 字符串
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << utf8 << std::endl;
    std::cout << "开源地址：https://github.com/drakmemory/OUCTool" << std::endl;
    std::cout << "暗忆QQ：1239124460" << std::endl;

    App app;
    return app.Run();
}
