#include "lua_windows.h"
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdint.h>

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

        if (GetModuleFileNameExA(process, NULL, mod_name, sizeof(mod_name)) > 0) {
            if (StrStrIA(mod_name, path)) {
                lua_pushinteger(L, pids[i]);
                result++;
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

BOOL CALLBACK enum_child_proc(HWND hwnd, LPARAM lparam)
{
    struct enum_wnd_param* param = (struct enum_wnd_param*)lparam;
    lua_pushlightuserdata(param->L, hwnd);
    param->count++;
    return TRUE;
}

static int lua_get_child_window(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;

    struct enum_wnd_param param = { 0 };
    param.L = L;

    EnumChildWindows(hwnd, enum_child_proc, (LPARAM)&param);
    return param.count;
}

static void lua_pushlwstring(lua_State* L, wchar_t* wstr, int len)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, len, NULL, 0, NULL, FALSE);
    if (size == 0) {
        lua_pushstring(L, "");
        return;
    }

    char* utf8str = (char*)malloc(size);
    int cnt = WideCharToMultiByte(CP_UTF8, 0, wstr, len, utf8str, size, NULL, NULL);
    if (cnt > 0) {
        lua_pushlstring(L, utf8str, cnt);
    } else {
        lua_pushstring(L, "");
    }
    free(utf8str);
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

static bool lua_toinput(lua_State* L, int idx, INPUT* input)
{
    do {
        if (!lua_istable(L, idx))
            break;

        if (lua_getfield(L, idx, "type") != LUA_TNUMBER)
            break;

        input->type = (DWORD)lua_tointeger(L, -1);
        if (input->type == INPUT_MOUSE) {
            if (lua_getfield(L, idx, "x") != LUA_TNUMBER)
                break;
            input->mi.dx = (LONG)lua_tointeger(L, -1);
            if (lua_getfield(L, idx, "y") != LUA_TNUMBER)
                break;
            input->mi.dy = (LONG)lua_tointeger(L, -1);
            if (lua_getfield(L, idx, "flags") != LUA_TNUMBER)
                break;
            input->mi.dwFlags = (DWORD)lua_tointeger(L, -1);
        } else if (input->type == INPUT_KEYBOARD) {
            if (lua_getfield(L, idx, "vk") != LUA_TNUMBER)
                break;
            input->ki.wVk = (WORD)lua_tointeger(L, -1);
            if (lua_getfield(L, idx, "flags") != LUA_TNUMBER)
                break;
            input->mi.dwFlags = (DWORD)lua_tointeger(L, -1);
        } else if (input->type == INPUT_HARDWARE) {
            // TODO
        } else {
            break;
        }
        return true;
    } while (0);
    return false;
}

static int lua_send_input(lua_State* L)
{
    int top = lua_gettop(L);
    INPUT* inputs = (INPUT*)calloc(top, sizeof(INPUT));

    if (!inputs)
        return luaL_error(L, "alloc memory failed");

    for (int i = 0; i < top; i++) {
        lua_settop(L, top);

        if (!lua_toinput(L, i + 1, inputs + i)) {
            free(inputs);
            return luaL_argerror(L, i + 1, "parameter invalid");
        }
    }
    UINT cnt = SendInput(top, inputs, sizeof(INPUT));
    lua_pushinteger(L, cnt);
    free(inputs);
    return 1;
}

static int lua_get_window_rect(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;
    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
        return 0;
    lua_pushinteger(L, rect.left);
    lua_pushinteger(L, rect.top);
    lua_pushinteger(L, rect.right);
    lua_pushinteger(L, rect.bottom);
    return 4;
}

static int lua_get_cursor_pos(lua_State* L)
{
    POINT point;
    if (!GetCursorPos(&point))
        return 0;
    lua_pushinteger(L, point.x);
    lua_pushinteger(L, point.y);
    return 2;
}

static int lua_set_cursor_pos(lua_State* L)
{
    int x = (int)lua_tointeger(L, 1);
    int y = (int)lua_tointeger(L, 2);

    BOOL ret = SetCursorPos(x, y);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_get_async_key_state(lua_State* L)
{
    int top = lua_gettop(L);
    for (int i = 0; i < top; i++) {
        int key = (int)lua_tointeger(L, i + 1);
        int down = GetAsyncKeyState(key) & 0x01;
        lua_pushboolean(L, down);
    }
    return top;
}

static int lua_show_window(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;
    int cmd = (int)lua_tointeger(L, 2);
    BOOL ret = ShowWindow(hwnd, cmd);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_set_console_title(lua_State* L)
{
    const char* title = lua_tostring(L, 1);
    if (!title)
        return 0;
    BOOL ret = SetConsoleTitleA(title);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_post_message(lua_State* L)
{
    HWND hwnd = lua_touserdata(L, 1);
    if (!hwnd)
        return 0;
    UINT msg = (UINT)lua_tointeger(L, 2);
    WPARAM wparam = (WPARAM)lua_tointeger(L, 3);
    LPARAM lparam = (LPARAM)lua_tointeger(L, 4);
    PostMessageA(hwnd, msg, wparam, lparam);
    return 0;
}

static int lua_kb_code(lua_State* L)
{
    size_t sz;
    const char* s = lua_tolstring(L, 1, &sz);
    if (sz == 0)
        return 0;

    SHORT key = VkKeyScanA(s[0]);
    UINT scan = MapVirtualKeyA(LOBYTE(key), 0);
    lua_pushinteger(L, key);
    lua_pushinteger(L, scan);
    return 2;
}

int luaopen_windows(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "find_process", lua_find_process },
        { "find_window", lua_find_window },
        { "get_child_window", lua_get_child_window },
        { "get_window_text", lua_get_window_text },
        { "set_foreground_window", lua_set_foreground_window },
        { "get_foreground_window", lua_get_foreground_window },
        { "send_input", lua_send_input },
        { "get_window_rect", lua_get_window_rect },
        { "get_cursor_pos", lua_get_cursor_pos },
        { "set_cursor_pos", lua_set_cursor_pos },
        { "get_async_key_state", lua_get_async_key_state },
        { "show_window", lua_show_window },
        { "set_console_title", lua_set_console_title },
        { "post_message", lua_post_message },
        { "kb_code", lua_kb_code },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}
