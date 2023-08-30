#include "lua_system.h"
#include <Windows.h>

struct enum_wnd_param {
    HWND hwnd;
    DWORD pid;
};

static BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam)
{
    struct enum_wnd_param* param = (struct enum_wnd_param*)lparam;
    DWORD pid;
    if (!GetWindowThreadProcessId(hwnd, &pid))
        return TRUE;

    if (pid != param->pid)
        return TRUE;

    param->hwnd = hwnd;
    return FALSE;
}

static int lua_find_window(lua_State* L)
{
    struct enum_wnd_param param;
    param.hwnd = NULL;
    param.pid = (DWORD)lua_tointeger(L, 1);

    EnumWindows(enum_wnd_proc, (LPARAM)&param);

    if (!param.hwnd)
        return 0;

    lua_pushlightuserdata(L, param.hwnd);
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

static int lua_keybd_event(lua_State* L)
{
    BYTE vk = (BYTE)lua_tointeger(L, 1);
    BYTE scan = (BYTE)lua_tointeger(L, 2);
    DWORD flags = (DWORD)lua_tointeger(L, 3);
    keybd_event(vk, scan, flags, 0);
    return 0;
}

int luaopen_system(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "find_window", lua_find_window },
        { "set_foreground_window", lua_set_foreground_window },
        { "keybd_event", lua_keybd_event },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}
