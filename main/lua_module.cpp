#include "lua_module.h"
#include "lua_plugin.h"
#include "unicode.h"

#ifdef WIN32
#include <Windows.h>
#include <iostream>

static inline void lua_writestring_utf8(const char* s, size_t l)
{
    try {
        auto str = utf_cvt().from_bytes(s, s + l);
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), str.c_str(), (DWORD)str.size(), nullptr, nullptr);
    } catch (...) {
        printf(s);
    }
}

static inline void lua_writeline_utf8() { WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, nullptr, nullptr); }

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
            lua_writestring_utf8("\t", 1);
        lua_writestring_utf8(s, l);
        lua_pop(L, 1); /* pop result */
    }
    lua_writeline_utf8();
    return 0;
}
#endif

bool lua_module::lua_create()
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
    return true;
}

bool lua_module::lua_init()
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

void lua_module::uninit()
{
    if (_L) {
        lua_close(_L);
        _L = nullptr;
    }
}

bool lua_module::call_lua_function(const char* file, const char* function, int arg_count, int result_count)
{
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

    return true;
}
