#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

extern "C" int luaopen_windows(lua_State* L);

inline void open_plugins(lua_State* L) { luaL_requiref(L, "windows", luaopen_windows, 0); }