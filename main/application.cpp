#include "application.h"
#include "unicode.h"

application::~application()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    _exit_flag = true;
    _module.uninit();
}

bool application::init(int argc, wchar_t** argv)
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    if (!_module.init())
        return false;

    auto L = _module.vm();
    lua_guard guard(L);

    for (int i = 1; i < argc; ++i) {
        auto str = utf_cvt().to_bytes(argv[i]);
        lua_pushlstring(L, str.c_str(), str.size());
    }

    if (!_module.call_lua_function("script/main.lua", "init", argc - 1, 0))
        return false;

    return true;
}

int application::run()
{
    while (!_exit_flag) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}

void application::set_exit() { _exit_flag = true; }

void application::tick()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    lua_guard guard(_module.vm());
    _module.call_lua_function("script/main.lua", "tick", 0, 0);
}
