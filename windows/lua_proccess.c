#include "lua_windows.h"
#include "unicode.h"
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdint.h>

#define LUA_PROCESS_MT ("com.dpull.lib.ProcessMT")

struct lua_process {
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION proc_info;

    HANDLE stdout_read;
    HANDLE stdout_write;
    HANDLE stderr_read;
    HANDLE stderr_write;
};

static int lua_process_destory(lua_State* L)
{
    struct lua_process* lua_process = (struct lua_process*)luaL_checkudata(L, 1, LUA_PROCESS_MT);
    if (!lua_process)
        return luaL_argerror(L, 1, "parameter self invalid");

    if (lua_process->proc_info.hProcess) {
        WaitForSingleObject(lua_process->proc_info.hProcess, INFINITE);
        CloseHandle(lua_process->proc_info.hProcess);
        lua_process->proc_info.hProcess = NULL;

        CloseHandle(lua_process->proc_info.hThread);
        lua_process->proc_info.hThread = NULL;
    }

    if (lua_process->stdout_read) {
        CloseHandle(lua_process->stdout_read);
        lua_process->stdout_read = NULL;
    }

    if (lua_process->stdout_write) {
        CloseHandle(lua_process->stdout_write);
        lua_process->stdout_write = NULL;
    }

    if (lua_process->stderr_read) {
        CloseHandle(lua_process->stderr_read);
        lua_process->stderr_read = NULL;
    }

    if (lua_process->stderr_write) {
        CloseHandle(lua_process->stderr_write);
        lua_process->stderr_write = NULL;
    }

    return 0;
}

static bool lua_read_file(lua_State* L, HANDLE h)
{
    DWORD available;
    if (!PeekNamedPipe(h, NULL, 0, NULL, &available, NULL))
        return false;

    if (available == 0)
        return false;

    void* buffer = malloc(available + 1);
    DWORD bytes_read = 0;
    if (ReadFile(h, buffer, available, &bytes_read, NULL) && bytes_read > 0) {
        lua_pushlstring(L, buffer, bytes_read);
    }
    free(buffer);
    return bytes_read != 0;
}

static int lua_process_read(lua_State* L)
{
    struct lua_process* lua_process = (struct lua_process*)luaL_checkudata(L, 1, LUA_PROCESS_MT);
    if (!lua_process)
        return luaL_argerror(L, 1, "parameter self invalid");

    if (!lua_read_file(L, lua_process->stdout_read)) {
        lua_pushstring(L, "");
    }

    if (!lua_read_file(L, lua_process->stderr_read)) {
        lua_pushstring(L, "");
    }

    return 2;
}

static int lua_process_get_exit_code(lua_State* L)
{
    struct lua_process* lua_process = (struct lua_process*)luaL_checkudata(L, 1, LUA_PROCESS_MT);
    if (!lua_process)
        return luaL_argerror(L, 1, "parameter self invalid");

    DWORD exit_code = STILL_ACTIVE;
    if (!GetExitCodeProcess(lua_process->proc_info.hProcess, &exit_code)) {
        return 0;
    }

    lua_pushboolean(L, exit_code != STILL_ACTIVE);
    lua_pushinteger(L, exit_code);
    return 2;
}

static int lua_process_terminate(lua_State* L)
{
    struct lua_process* lua_process = (struct lua_process*)luaL_checkudata(L, 1, LUA_PROCESS_MT);
    if (!lua_process)
        return luaL_argerror(L, 1, "parameter self invalid");

    BOOL succ = TerminateProcess(lua_process->proc_info.hProcess, 1);
    lua_pushboolean(L, succ);
    return 1;
}

luaL_Reg lua_process_funcs[] = {
    { "__gc", lua_process_destory }, 
    { "read", lua_process_read }, 
    { "get_exit_code", lua_process_get_exit_code }, 
    { "terminate", lua_process_terminate },
    { NULL, NULL }
};

bool lua_create_process_meta_init(lua_State* L)
{
    if (luaL_newmetatable(L, LUA_PROCESS_MT)) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_setfuncs(L, lua_process_funcs, 0);
        lua_pop(L, 1);

        return true;
    }
    return false;
}

static bool create_pipe(PHANDLE read_pipe, PHANDLE write_pipe)
{
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(read_pipe, write_pipe, &sa, 0))
        return false;

    SetHandleInformation(*read_pipe, HANDLE_FLAG_INHERIT, 0);
    return true;
}

int lua_create_process(lua_State* L)
{
    const char* cmd = lua_tostring(L, 1);
    if (!cmd)
        return luaL_argerror(L, 1, "parameter cmd invalid");

    const char* work_dir = NULL;
    int top = lua_gettop(L);
    if (top > 1) {
        work_dir = lua_tostring(L, 2);
        if (!work_dir)
            return luaL_argerror(L, 2, "parameter work_dir invalid");
    }

    struct lua_process* lua_process = (struct lua_process*)lua_newuserdata(L, sizeof(*lua_process));
    memset(lua_process, 0, sizeof(*lua_process));

    luaL_getmetatable(L, LUA_PROCESS_MT);
    lua_setmetatable(L, -2);

    if (!create_pipe(&lua_process->stdout_read, &lua_process->stdout_write))
        return luaL_error(L, "create stdout pipe failed");

    if (!create_pipe(&lua_process->stderr_read, &lua_process->stderr_write))
        return luaL_error(L, "create stderr pipe failed");

    lua_process->startup_info.cb = sizeof(lua_process->startup_info);
    lua_process->startup_info.hStdOutput = lua_process->stdout_write;
    lua_process->startup_info.hStdError = lua_process->stderr_write;
    lua_process->startup_info.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, work_dir, &lua_process->startup_info, &lua_process->proc_info)) {
        DWORD error_code = GetLastError();
        return luaL_error(L, "create process failed, error_code:%d", error_code);
    }
    return 1;
}
