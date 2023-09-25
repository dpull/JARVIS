#pragma once

#include "application.h"

class application_windows : public application {
public:
    bool init(int argc, wchar_t** argv) override;
    int run() override;

private:
    void tick_cmd();
    void change_enable();
};