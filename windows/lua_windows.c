#include "lua_windows.h"
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

static int lua_find_process(lua_State* L)
{
    const char* path = lua_tostring(L, 1);
    DWORD pids[4096], needed, count, result;
    char mod_name[MAX_PATH];

    if (!EnumProcesses(pids, sizeof(pids), &needed))
        return 0;

    count = needed / sizeof(DWORD);
    result = 0;

    for (DWORD i = 0; i < count; i++) {
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
        if (!process)
            continue;

        HMODULE mod;
        if (EnumProcessModules(process, &mod, sizeof(mod), &needed)) {
            if (GetModuleBaseNameA(process, mod, mod_name, sizeof(mod_name))) {
                printf("%s\n", mod_name);
                if (StrStrIA(mod_name, path)) {
                    lua_pushinteger(L, pids[i]);
                    result++;
                }
            }
        }
        CloseHandle(process);
    }
    return result;
}

struct enum_wnd_param {
    lua_State* L;
    int count;
    DWORD pid;
};

static BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam)
{
    struct enum_wnd_param* param = (struct enum_wnd_param*)lparam;
    DWORD pid;
    if (GetWindowThreadProcessId(hwnd, &pid) && pid == param->pid) {
        lua_pushlightuserdata(param->L, hwnd);
        param->count++;
    }
    return TRUE;
}

static int lua_find_window(lua_State* L)
{
    struct enum_wnd_param param;
    param.L = L;
    param.count = 0;
    param.pid = (DWORD)lua_tointeger(L, 1);

    EnumWindows(enum_wnd_proc, (LPARAM)&param);

    return param.count;
}

static void lua_pushlwstring(lua_State* L, wchar_t* wstr, int len)
{
    char utf8str[4096];
    int cnt = WideCharToMultiByte(CP_UTF8, 0, wstr, len, utf8str, sizeof(utf8str), NULL, NULL);
    if (cnt > 0) {
        lua_pushlstring(L, utf8str, cnt);
    } else {
        lua_pushstring(L, "");
    }
}

static int lua_get_window_text(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;

    int len = GetWindowTextLengthW(hwnd) + 1;
    wchar_t* str = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!str)
        return 0;

    int cnt = GetWindowTextW(hwnd, str, len);
    lua_pushlwstring(L, str, cnt);
    free(str);
    return 1;
}

static int lua_set_foreground_window(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;
    BOOL ret = SetForegroundWindow(hwnd);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_get_foreground_window(lua_State* L)
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd)
        return 0;
    lua_pushlightuserdata(L, hwnd);
    return 1;
}

static int lua_keybd_event(lua_State* L)
{
    BYTE vk = (BYTE)lua_tointeger(L, 1);
    BYTE scan = (BYTE)lua_tointeger(L, 2);
    DWORD flags = (DWORD)lua_tointeger(L, 3);
    keybd_event(vk, scan, flags, 0);
    return 0;
}

int luaopen_windows(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "find_process", lua_find_process },
        { "find_window", lua_find_window },
        { "get_window_text", lua_get_window_text },
        { "set_foreground_window", lua_set_foreground_window },
        { "get_foreground_window", lua_get_foreground_window },
        { "keybd_event", lua_keybd_event },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}
