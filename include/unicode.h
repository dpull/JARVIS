#pragma once

#ifdef __cplusplus
#include <codecvt>
using utf_cvt_t = std::wstring_convert<std::codecvt_utf8<wchar_t>>;
utf_cvt_t& utf_cvt();
#endif

#ifdef __cplusplus
extern "C" {
#endif

int utf8_utf16(const char* utf8, wchar_t utf16[], size_t size);
int utf16_utf8(const wchar_t* utf16, char utf8[], size_t size);

#ifdef __cplusplus
}
#endif
