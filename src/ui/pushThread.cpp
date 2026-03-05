// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/pushThread.h"

#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/Events.h"

#include "push/PushCore.h"
#include "push/PushContext.h"
#include "ui/push/MainWidget.h" //does not belong here...

#include "slr_config.h"
#include "logger.h"

#include <thread>
#include <atomic>
#include <memory>
#include <chrono>

namespace PushThread {

std::unique_ptr<PushLib::PushCore> _pushDev;
std::unique_ptr<PushUI::MainWidget> _pushMainWidget;

std::thread _pushRunner;
std::atomic<bool> _threadRunning;

void threadLoop(slr::MidiPort * port);

void start(slr::MidiPort * port) {
    if(_threadRunning) {
        LOG_WARN("Push thread already running");
        return;
    }

    _pushRunner = std::thread(&threadLoop, port);
}

void shutdown() {
    _pushDev->disconnect();

    _threadRunning = false;
    _pushRunner.join();

    _pushDev.reset();
}

bool isRunning() {
    return _threadRunning;
}

void threadLoop(slr::MidiPort *port) {
    _pushDev.reset();
    _pushMainWidget.reset();

    _pushDev = std::make_unique<PushLib::PushCore>();
    PushLib::PushContext *pctx = _pushDev->context();
    _pushMainWidget = std::make_unique<PushUI::MainWidget>(pctx);

    _pushDev->setMainWidget(_pushMainWidget.get());

    if(!_pushDev->connect(port)) {
        LOG_ERROR("Failed to connect Push");
        return;
    }

    LOG_INFO("Push Inited successfully");
    _threadRunning = true;

    int FPS = 30.0;
    std::chrono::milliseconds dt(1000/FPS);
    std::chrono::time_point<std::chrono::steady_clock> lastFrame = std::chrono::steady_clock::now();
    while(_threadRunning) {
        if(_pushDev->connected()) {
            auto now = std::chrono::steady_clock::now();

            unsigned long diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame).count();
            _pushDev->tick(diff);

            lastFrame = now;
        } else {
            //try reconnect
            //ugh... than midi would be disconnected as well, need some mechanism in MidiController to reconnect midi
        }

        std::this_thread::sleep_for(dt);
    }
}

}