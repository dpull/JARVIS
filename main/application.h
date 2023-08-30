#pragma once

#include <mutex>

class application {
public:
    application();
    ~application();

    bool init(int argc, char** argv);
    int run();
    void set_exit();

    bool exec(const char* file);

private:
    bool lua_create();
    bool lua_init();
    bool call_lua_function(const char* file, const char* function, int arg_count, int result_count);

private:
    std::mutex _mutex;
    volatile int _exit_flag = false;
    struct lua_State* _L = nullptr;
    int _lua_func_ref;
};
