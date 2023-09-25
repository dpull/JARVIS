#pragma once

#include "lua_module.h"
#include <mutex>

class application {
public:
    virtual ~application();

    virtual bool init(int argc, wchar_t** argv);
    virtual int run();

    void set_exit();

protected:
    void tick();

protected:
    std::mutex _mutex;
    lua_module _module;
    volatile int _exit_flag = false;
};
