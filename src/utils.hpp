#pragma once
#include <windows.h>
#include <string>
#include <vector>

// UTF-8 转 GBK
inline std::string Utf8ToGbk(const std::string& utf8Str) {
    if (utf8Str.empty()) return "";

    // 1. UTF-8 转 UTF-16 (宽字符)
    int utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (utf16Len <= 0) return "";

    std::vector<wchar_t> utf16(utf16Len);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, utf16.data(), utf16Len);

    // 2. UTF-16 转 GBK
    int gbkLen = WideCharToMultiByte(CP_ACP, 0, utf16.data(), -1, nullptr, 0, nullptr, nullptr);
    if (gbkLen <= 0) return "";

    std::string gbkStr(gbkLen - 1, '\0');  // -1 排除 null 终止符
    WideCharToMultiByte(CP_ACP, 0, utf16.data(), -1, &gbkStr[0], gbkLen, nullptr, nullptr);

    return gbkStr;
}

// UTF-16 (wstring) 转 UTF-8
inline std::string Utf16ToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return std::string();
    std::string result(size_needed - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size_needed, nullptr, nullptr);
    return result;
}