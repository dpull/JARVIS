#include "application.h"
#include "lua_plugin.h"
#include <cassert>
#include <thread>

#ifdef WIN32
#include <Windows.h>

static void lua_writestring_utf8(const char* s, size_t l)
{
    wchar_t* wbuf = nullptr;
    char* mbuf = nullptr;
    bool succeed = false;

    do {
        int wsize = MultiByteToWideChar(CP_UTF8, 0, s, (int)l, NULL, 0);
        if (wsize == 0)
            break;

        wbuf = new wchar_t[wsize];
        if (!wbuf)
            break;

        if (MultiByteToWideChar(CP_UTF8, 0, s, (int)l, wbuf, wsize) == 0)
            break;

        int msize = WideCharToMultiByte(CP_ACP, 0, wbuf, wsize, NULL, 0, NULL, FALSE);
        if (msize == 0)
            break;

        mbuf = new char[msize];
        if (!mbuf)
            break;

        if (WideCharToMultiByte(CP_ACP, 0, wbuf, wsize, mbuf, msize, NULL, FALSE) == 0)
            break;

        fwrite(mbuf, sizeof(char), msize, stdout);
        succeed = true;

    } while (0);

    if (wbuf)
        delete[] wbuf;
    if (mbuf)
        delete[] mbuf;
    if (!succeed)
        fwrite(s, sizeof(char), l, stdout);
}

// modify from luaB_print
static int luaB_print_utf8(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i = 1; i <= n; i++) {
        const char* s;
        size_t l;
        lua_pushvalue(L, -1); /* function to be called */
        lua_pushvalue(L, i); /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l); /* get result */
        if (s == NULL)
            return luaL_error(L, "'tostring' must return a string to 'print'");
        if (i > 1)
            lua_writestring("\t", 1);
        lua_writestring_utf8(s, l);
        lua_pop(L, 1); /* pop result */
    }
    lua_writeline();
    return 0;
}
#endif

static int lua_sleep_ms(lua_State* L)
{
    int ms = (int)lua_tointeger(L, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return 0;
}

application::application()
    : _lua_func_ref(LUA_NOREF)
{
}

application::~application()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    _exit_flag = true;

    if (_L) {
        lua_close(_L);
        _L = nullptr;
    }
}

bool application::init(int argc, char** argv)
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    if (!lua_create())
        return false;

    if (!lua_init())
        return false;

    for (int i = 1; i < argc; ++i) {
        lua_pushstring(_L, argv[i]);
    }

    if (!call_lua_function("script/main.lua", "init", argc - 1, 0))
        return false;

    return true;
}

bool application::lua_init()
{
    const char* LuaScript = R"(
        local script = ...
        local fn, msg = loadfile(script)
        assert(fn, msg)
        return fn()
    )";

    auto top = lua_gettop(_L);
    auto err = luaL_loadstring(_L, LuaScript);
    if (err != LUA_OK) {
        printf("lua_init, luaL_loadstring failed.%s", lua_tostring(_L, -1));
        return false;
    }

    lua_pushstring(_L, "script/preload.lua");
    err = lua_pcall(_L, 1, 1, 0);
    if (err != LUA_OK) {
        printf("lua_init, lua_pcall failed.%s", lua_tostring(_L, -1));
        return false;
    }

    auto ref = luaL_ref(_L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL || ref == LUA_NOREF) {
        printf("lua_init, luaL_ref failed.");
        return false;
    }

    _lua_func_ref = ref;
    lua_settop(_L, top);
    return true;
}

bool application::call_lua_function(const char* file, const char* function, int arg_count, int result_count)
{
    int top = lua_gettop(_L) - arg_count;

    lua_pushstring(_L, function);
    lua_insert(_L, -1 - arg_count);
    arg_count++;

    lua_pushstring(_L, file);
    lua_insert(_L, -1 - arg_count);
    arg_count++;

    lua_rawgeti(_L, LUA_REGISTRYINDEX, _lua_func_ref);
    assert(lua_isfunction(_L, -1));
    lua_insert(_L, -1 - arg_count);

    int ret = lua_pcall(_L, arg_count, result_count, 0);
    if (ret != LUA_OK) {
        printf("CallModuleFunction, [%s]%s failed.%s", file, function, lua_tostring(_L, -1));
        return false;
    }

    lua_settop(_L, top);
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

bool application::exec(const char* file)
{
    if (!lua_create())
        return false;

    auto status = luaL_loadfile(_L, file);
    if (status) {
        fprintf(stderr, "couldn't load %s:%s\n", file, lua_tostring(_L, -1));
        return false;
    }

    int ret = lua_pcall(_L, 0, 0, 0);
    if (ret != LUA_OK) {
        fprintf(stderr, "call %s failed:%s", file, lua_tostring(_L, -1));
        return false;
    }

    return true;
}

void application::tick()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    call_lua_function("script/main.lua", "tick", 0, 0);
}

bool application::lua_create()
{
    _L = luaL_newstate();
    if (!_L)
        return false;
    open_plugins(_L);
    luaL_openlibs(_L);
#ifdef WIN32
    lua_pushcfunction(_L, luaB_print_utf8);
    lua_setglobal(_L, "print");
#endif
    lua_pushcfunction(_L, lua_sleep_ms);
    lua_setglobal(_L, "sleep_ms");
    return true;
}
