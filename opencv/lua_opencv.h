#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

int luaopen_opencv(lua_State* L);
}
