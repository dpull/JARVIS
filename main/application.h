#pragma once

#include <mutex>

class application {
public:
    application();
    virtual  ~application();

    virtual bool init(int argc, char** argv);
    virtual int run();

    void set_exit();
    bool exec(const char* file);

protected:
    void tick();
    bool lua_create();
    bool lua_init();
    bool call_lua_function(const char* file, const char* function, int arg_count, int result_count);

protected:
    std::mutex _mutex;
    volatile int _exit_flag = false;
    struct lua_State* _L = nullptr;
    int _lua_func_ref;
};
