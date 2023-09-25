#include "unicode.h"

thread_local utf_cvt_t* wstring_convert = nullptr;

utf_cvt_t& utf_cvt()
{ 
    if (!wstring_convert)
        wstring_convert = new utf_cvt_t;
    return *wstring_convert; 
}

int utf8_utf16(const char* utf8, wchar_t utf16[], size_t size)
{
    auto str = utf_cvt().from_bytes(utf8);
    auto len = static_cast<int>(str.size());
    if (len + 1 > size) {
        return -len;
    }
    memcpy(utf16, str.c_str(), len * sizeof(wchar_t));
    utf16[len] = L'\0';
    return len;
}

int utf16_utf8(const wchar_t* utf16, char utf8[], size_t size)
{
    auto str = utf_cvt().to_bytes(utf16);
    auto len = static_cast<int>(str.size());
    if (len + 1 > size) {
        return -len;
    }
    memcpy(utf8, str.c_str(), len * sizeof(char));
    utf8[len] = '\0';
    return len;
}
