// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <stdlib.h>

#include "display/guiThread.h"
#include "core/ControlEngine.h"
#include "common/logger.h"

std::atomic<bool> _shutdown;

void handle_sigint(int i) {
    (void)i;
    
    // shutdown();
    _shutdown = true;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    loguru::init(argc, argv);
    loguru::add_file("log.txt", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::g_preamble_date = false;
    loguru::g_preamble_time = false;
    loguru::g_preamble_thread = false;

    signal(SIGINT, handle_sigint);

    LOG_INFO("C++ version is %li", __cplusplus);

    _shutdown = false;

    initGui();

    if(!slr::ControlEngine::init(_shutdown)) {
        LOG_ERROR("Failed at startup!");
        return 1;
    }
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    runGui(_shutdown);

    if(!slr::ControlEngine::shutdown()) {
        LOG_ERROR("Failed at shutdown");
    }

    return 0;
}
