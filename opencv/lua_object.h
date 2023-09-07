#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

template <typename T> struct lua_object {
    static bool newmetatable(lua_State* L, luaL_Reg* l = nullptr)
    {
        if (!luaL_newmetatable(L, mt_name()))
            return false;

        luaL_Reg defaults[] = { { "__gc", gc }, { nullptr, nullptr } };
        l = l ? l : defaults;

        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_setfuncs(L, l, 0);
        lua_pop(L, 1);
        return true;
    }

    template <typename... Args> static T* alloc(lua_State* L, Args&&... args)
    {
        auto mem = lua_newuserdata(L, sizeof(T));
        luaL_getmetatable(L, mt_name());
        lua_setmetatable(L, -2);

        auto obj = new (mem) T(std::forward<Args>(args)...);
        return obj;
    }

    static T* toobj(lua_State* L, int idx)
    {
        auto name = typeid(T).name();
        return static_cast<T*>(luaL_checkudata(L, idx, name));
    }

    static const char* mt_name()
    {
        auto name = typeid(T).name();
        return name;
    }

    static int gc(lua_State* L)
    {
        auto obj = toobj(L, 1);
        if (!obj)
            return luaL_argerror(L, 1, "parameter self invalid");
        obj->~T();
        return 0;
    }
};
