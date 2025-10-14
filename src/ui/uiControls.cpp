// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/uiControls.h"
#include "ui/display/MainWindow.h"
#include "ui/display/RouteManager.h"

#include <cassert>
#include <functional>

#include <atomic>
#include <deque>
#include <mutex>
#include <memory>
#include <functional>
#include <lvgl.h>

std::mutex g_async_mtx;
std::deque< std::unique_ptr<std::function<void()>> > g_async_queue;
std::atomic_bool g_dispatch_scheduled{false};

namespace UIControls {

void postToLvgl(std::function<void()> fn) {
     // TODO: frequently and periodically called functions such as updatePlayheadPostion
        // must use another mechanism for updating gui
        // check gdrive->feature->uicontrols and periodic updates for more detailed info
    
    auto p = std::make_unique<std::function<void()>>(std::move(fn));
    bool need_schedule = false;
    {
        std::lock_guard<std::mutex> lk(g_async_mtx);
        g_async_queue.push_back(std::move(p));
        
        if (!g_dispatch_scheduled.exchange(true)) {
            need_schedule = true;
        }
    }

    if (need_schedule) {
        // lv_async_call(lv_async_dispatcher, nullptr);
    }
}

/*
void postToLvgl(std::function<void()> fn) {
    static auto lambda = [](void *userData) {
        auto fn = static_cast<std::function<void()>*>(userData);
        (*fn)();
        delete fn;
    };

    auto * ptr = new std::function<void()>(std::move(fn));

    //no, lock\unlock not helping, still getting corrupted user_data somehow
    lv_lock();
    lv_async_call(lambda, ptr);
    lv_unlock();
} */

void floatingInfo(std::string text) {
    postToLvgl([text]() {
        UI::MainWindow::inst()->floatingTextRegular(text);
    });
}

void floatingWarning(std::string text) {
    postToLvgl([text]() {
        UI::MainWindow::inst()->floatingTextWarning(text);
    });
}

/* Module Related */
void addModuleUI(const slr::Module * mod, slr::AudioUnitView * view) {
    postToLvgl([mod, view]() {
        UI::MainWindow::inst()->createUI(mod, view);
    });
}

void updateModuleUI(slr::ID id) {
    postToLvgl([id]() {
        UI::MainWindow::inst()->updateUI(id);
    });
}

void destroyModuleUI(slr::ID id) {
    postToLvgl([id]() {
        UI::MainWindow::inst()->destroyUI(id);
    });
}


/* Timeline */
//if Time Signature or BPM updated pass true, otherwise false
void updateTimeline(const bool timeSigOrBpm) {
    postToLvgl([timeSigOrBpm]() {
        UI::MainWindow::inst()->updateTimeline(timeSigOrBpm);
    });
}

void updatePlayheadPosition(slr::frame_t position) {
    postToLvgl([position]() {
        UI::MainWindow::inst()->updatePlayheadPosition(position);
    });
}

/* Route Manager */
void updateRouteManager() {
    postToLvgl([]() {
        UI::MainWindow::inst()->_routeManager->liveUpdate();
    });
}

/* Metronome */
void updateMetronomeState(bool onoff) {
    postToLvgl([onoff]() {
        UI::MainWindow::inst()->updateMetronomeState(onoff);
    });
}


} //namespace UIControls