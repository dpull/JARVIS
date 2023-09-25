#pragma once
#include "lua.hpp"
#include <cassert>

class lua_guard {
public:
    lua_guard(lua_State* L)
        : _L(L)
    {
        _top = lua_gettop(L);
    }
    ~lua_guard() { lua_settop(_L, _top); }

private:
    lua_guard(const lua_guard& other) = delete;
    lua_guard(lua_guard&& other) = delete;
    lua_guard& operator=(const lua_guard&) = delete;

private:
    int _top = 0;
    lua_State* _L = nullptr;
};

class lua_module {
public:
    ~lua_module() { assert(_L == nullptr); }
    bool init() { return lua_create() && lua_init(); }
    void uninit();

    lua_State* vm() { return _L; }
    bool call_lua_function(const char* file, const char* function, int arg_count, int result_count);

private:
    bool lua_create();
    bool lua_init();

    lua_State* _L = nullptr;
    int _lua_func_ref = LUA_NOREF;
};
