// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <stdlib.h>

#include "ui/guiThread.h"
#include "core/ControlEngine.h"
#include "logger.h"

void handle_sigint(int i) {
    (void)i;
    
    shutdown();
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    loguru::init(argc, argv);
    loguru::add_file("log.txt", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::g_preamble_date = false;
    loguru::g_preamble_time = false;
    loguru::g_preamble_thread = false;
    // signal(SIGSEGV, handle_sigint);
    // signal(SIGABRT, handle_sigint);
    // signal(SIGFPE, handle_sigint);
    // signal(SIGILL, handle_sigint);
    // signal(SIGBUS, handle_sigint);

    signal(SIGINT, handle_sigint);

    initGui();

    if(!slr::ControlEngine::init()) {
        LOG_ERROR("Failed at startup!");
        return 1;
    }
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    runGui();

    if(!slr::ControlEngine::shutdown()) {
        LOG_ERROR("Failed at shutdown");
    }

    return 0;
}
