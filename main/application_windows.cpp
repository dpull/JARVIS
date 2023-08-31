#include "application_windows.h"
#include "lua_plugin.h"
#include <conio.h>
#include <Windows.h>

enum class event_id {
    hotKey_enable = 1,
    tick_timer,
};

bool application_windows::init(int argc, char** argv) {
    if (!RegisterHotKey(NULL, (int)event_id::hotKey_enable, MOD_CONTROL, VK_F12))
        return false;

    if (!SetTimer(NULL, (int)event_id::tick_timer, 50, NULL))
        return false;

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
            if (!_disable)
                tick();
            break;
        case WM_HOTKEY:
            _disable = !_disable;
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
    lua_pushstring(_L, cmd);
    call_lua_function("script/main.lua", "exec", 1, 0);
}
