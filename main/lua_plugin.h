#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

extern "C" int luaopen_system(lua_State* L);

inline void open_plugins(lua_State* L) { luaL_requiref(L, "system", luaopen_system, 0); }