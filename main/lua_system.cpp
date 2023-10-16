#include "lua_plugin.h"
#include <filesystem>
#include <thread>

static int lua_sleep_ms(lua_State* L)
{
    int ms = (int)lua_tointeger(L, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return 0;
}

static int lua_get_time_ms(lua_State* L)
{
    auto ms = std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000 / 1000;
    lua_pushinteger(L, ms);
    return 1;
}

static void traverse_directory(lua_State* L, int& array_cnt, const std::filesystem::path& path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            traverse_directory(L, array_cnt, entry.path());
        } else if (entry.is_regular_file()) {
            auto file_path = entry.path().u8string();

            lua_pushlstring(L, file_path.c_str(), file_path.size());
            array_cnt++;
            lua_rawseti(L, -2, array_cnt);
        }
    }
}

static int lua_get_directory_files(lua_State* L)
{
    auto path = lua_tostring(L, 1);
    int array_cnt = 0;
    lua_newtable(L);
    traverse_directory(L, array_cnt, std::filesystem::u8path(path));
    return 1;
}

static int lua_get_file_last_write_time(lua_State* L)
{
    auto path = lua_tostring(L, 1);
    auto mod_time = std::filesystem::last_write_time(std::filesystem::u8path(path));
    auto mod_time_s = std::chrono::time_point_cast<std::chrono::seconds>(mod_time);
    auto mod_time_t = mod_time_s.time_since_epoch().count();

    lua_pushinteger(L, mod_time_t);
    return 1;
}

int luaopen_system(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "sleep_ms", lua_sleep_ms },
        { "get_time_ms", lua_get_time_ms },
        { "get_directory_files", lua_get_directory_files },
        { "get_file_last_write_time", lua_get_file_last_write_time },

        { nullptr, nullptr },
    };

    luaL_newlib(L, l);

    return 1;
}
