#include "lua_async_task.h"
#include "lua_object.hpp"
#include "lua_plugin.h"

static void copy_data(std::vector<char>& dst, const char* src, size_t len)
{
    dst.resize(len);
    memcpy(dst.data(), src, len);
}

async_task::~async_task()
{
    _exit_flag.store(true);
    _cond_var.notify_one();
    _thread.join();
    _module.uninit();
}

bool async_task::init()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    if (!_module.init())
        return false;

    _thread = std::thread(&async_task::work_thread, this);
    return true;
}

int async_task::push(task* t)
{
    auto token = ++_token;
    t->token = token;
    {
        std::lock_guard<decltype(_mutex)> lock(_mutex);
        _request.push_back(t);
    }
    _cond_var.notify_one();
    return token;
}

task* async_task::pop()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    if (_response.empty())
        return nullptr;
    auto t = _response.front();
    _response.pop_front();
    return t;
}

void async_task::cancel_all()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    for (auto t : _request) {
        t->state = task_state::cancelled;
        _response.push_back(t);
    }
    _request.clear();
}

void async_task::work_thread()
{
    while (!_exit_flag.load()) {
        task* t = pop_request();
        if (!t) {
            lua_gc(_module.vm(), LUA_GCCOLLECT, 0);

            std::unique_lock lock(_mutex);
            if (!_request.empty())
                continue;

            _cond_var.wait(lock);
            continue;
        }

        exec(t);
        push_response(t);
    }
}

task* async_task::pop_request()
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    if (_request.empty())
        return nullptr;
    auto t = _request.front();
    _request.pop_front();
    return t;
}

void async_task::push_response(task* t)
{
    std::lock_guard<decltype(_mutex)> lock(_mutex);
    _response.push_back(t);
}

void async_task::exec(task* t)
{
    auto L = _module.vm();
    lua_guard guard(L);

    lua_pushlstring(L, t->request.data(), t->request.size());
    if (!_module.call_lua_function(t->file.c_str(), t->function.c_str(), 1, 1)) {
        t->state = task_state::error;
        return;
    }

    if (lua_isstring(L, -1)) {
        size_t len;
        auto resp = lua_tolstring(L, -1, &len);
        copy_data(t->response, resp, len);
    }
    t->state = task_state::completed;
}

static int lua_create_async_task(lua_State* L)
{
    auto at = lua_object<async_task>::alloc(L);
    if (!at->init())
        return 0;
    return 1;
}

static int lua_async_task_push(lua_State* L)
{
    auto at = lua_object<async_task>::toobj(L, 1);
    if (!at)
        return luaL_argerror(L, 1, "parameter self invalid");
    auto file = lua_tostring(L, 2);
    auto function = lua_tostring(L, 3);
    size_t len;
    auto req = lua_tolstring(L, 4, &len);

    auto t = new task;
    t->file = file;
    t->function = function;
    copy_data(t->request, req, len);
    auto token = at->push(t);
    lua_pushinteger(L, token);
    return 1;
}

static int lua_async_task_pop(lua_State* L)
{
    auto at = lua_object<async_task>::toobj(L, 1);
    if (!at)
        return luaL_argerror(L, 1, "parameter self invalid");

    auto t = at->pop();
    if (!t)
        return 0;

    lua_pushinteger(L, t->token);
    lua_pushinteger(L, static_cast<int>(t->state));
    if (!t->response.empty())
        lua_pushlstring(L, t->response.data(), t->response.size());
    else
        lua_pushnil(L);
    delete t;
    return 3;
}

int luaopen_async_task(lua_State* L)
{
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "create", lua_create_async_task },
        { nullptr, nullptr },
    };

    luaL_newlib(L, l);

    luaL_Reg m[] = {
        { "__gc", lua_object<async_task>::gc },
        { "push", lua_async_task_push },
        { "pop", lua_async_task_pop },
        { nullptr, nullptr },
    };
    lua_object<async_task>::newmetatable(L, m);

    return 1;
}
