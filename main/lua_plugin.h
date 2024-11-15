#pragma once

#include "lua.hpp"

extern "C" {

int luaopen_system(lua_State* L);
int luaopen_serialize(lua_State* L);
int luaopen_async_task(lua_State* L);
int luaopen_webclient(lua_State* L);

int luaopen_windows(lua_State* L);
#ifdef ENABLE_OPENCV
int luaopen_opencv(lua_State* L);
#endif
}

inline void open_plugins(lua_State* L)
{
    luaL_requiref(L, "system", luaopen_system, 0);
    luaL_requiref(L, "serialize", luaopen_serialize, 0);
    luaL_requiref(L, "async_task", luaopen_async_task, 0);
    luaL_requiref(L, "webclient", luaopen_webclient, 0);

    luaL_requiref(L, "windows", luaopen_windows, 0);
#ifdef ENABLE_OPENCV
    luaL_requiref(L, "opencv", luaopen_opencv, 0);
#endif
}