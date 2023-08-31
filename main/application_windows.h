#pragma once

#include "application.h"

class application_windows : public application {
public:
    bool init(int argc, char** argv) override;
    int run() override;

private:
    void tick_cmd();

private:
    bool _disable = false;
};