#pragma once
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include <stdbool.h>

int luaopen_windows(lua_State* L);

bool lua_create_process_meta_init(lua_State* L);
int lua_create_process(lua_State* L);