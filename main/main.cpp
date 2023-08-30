#include "application.h"
#include <memory>
#include <stdio.h>

std::unique_ptr<application> g_application = nullptr;
static bool SetConsoleInfo();

int main(int argc, char** argv)
{
    g_application.reset(new application());
    if (!g_application->init(argc, argv)) {
        printf("application init  failed.");
        return 1;
    }

    SetConsoleInfo();
    return g_application->run();
}

#ifndef WIN32
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

static void OnQuitSignal(int nSignal) { g_application->set_exit(); }

static void OnAbortSignal(int nSignal)
{
    signal(nSignal, SIG_DFL);
    kill(getpid(), nSignal);
}

static bool SetConsoleInfo()
{
    signal(SIGINT, OnQuitSignal);
    signal(SIGQUIT, OnQuitSignal);
    signal(SIGTERM, OnQuitSignal);
    return true;
}

#else
#include <Windows.h>

static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    g_application->set_exit();
    return true;
}

static bool SetConsoleInfo()
{
    SetConsoleCtrlHandler(&ConsoleHandlerRoutine, true);
    return true;
}

#endif
