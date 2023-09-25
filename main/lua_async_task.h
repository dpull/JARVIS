#pragma once
#include "lua_module.h"
#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

enum class task_state { pending, completed, cancelled, error };

struct task {
    // in
    std::string file;
    std::string function;
    std::vector<char> request;

    // out
    std::vector<char> response;
    int token = 0;
    task_state state = task_state::pending;
};

class async_task {
public:
    ~async_task();
    bool init();
    int push(task* t);
    task* pop();
    void cancel_all();

private:
    void work_thread();
    task* pop_request();
    void push_response(task* t);
    void exec(task* t);

private:
    lua_module _module;
    std::atomic<bool> _exit_flag = false;
    std::atomic<int> _token = 0;
    std::mutex _mutex;
    std::condition_variable _cond_var;
    std::thread _thread;
    std::list<task*> _request;
    std::list<task*> _response;
};
