#include "application_windows.h"
#include "lua_plugin.h"
#include <Windows.h>
#include <conio.h>

enum class event_id {
    hotKey_enable = 1,
    tick_timer,
};

bool application_windows::init(int argc, char** argv)
{
    SetProcessDPIAware();

    if (!RegisterHotKey(NULL, (int)event_id::hotKey_enable, MOD_CONTROL, VK_F12))
        return false;

    if (!SetTimer(NULL, (int)event_id::tick_timer, 100, NULL))
        return false;

    auto console = GetConsoleWindow();
    SetWindowPos(console, 0, 1, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    return application::init(argc, argv);
}

int application_windows::run()
{
    MSG msg;
    while (!_exit_flag) {
        if (!GetMessage(&msg, NULL, 0, 0))
            break;

        switch (msg.message) {
        case WM_TIMER:
            tick_cmd();
            tick();
            break;
        case WM_HOTKEY:
            change_enable();
            break;
        default:
            printf("application_windows unknown message:%d\n", msg.message);
        }
    }
    return 0;
}

void application_windows::tick_cmd()
{
    if (!_kbhit())
        return;
    char cmd[2] = { 0 };
    cmd[0] = _getch();

    std::lock_guard<decltype(_mutex)> lock(_mutex);
    lua_pushstring(_L, "cmd");
    lua_pushstring(_L, cmd);
    call_lua_function("script/main.lua", "on_event", 2, 0);
}

void application_windows::change_enable()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    lua_pushstring(_L, "change_enable");
    call_lua_function("script/main.lua", "on_event", 1, 0);
}
