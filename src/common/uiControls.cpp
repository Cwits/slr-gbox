// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/uiControls.h"
#include "display/elements/RootWindow.h"
#include "display/popups/RouteManager.h"
#include "display/popups/ModEngineTargetManager.h"

#include "push/pushThread.h"
#include "push/elements/RootWidget.h"

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

void floatingInfo(std::string text) {
    postToLvgl([text]() {
        UI::RootWindow::inst()->floatingText(false, text);
    });
}

void floatingWarning(std::string text) {
    postToLvgl([text]() {
        UI::RootWindow::inst()->floatingText(true, text);
    });
}

/* Module Related */
void addUnitUI(const slr::UnitDescriptor * desc, const std::shared_ptr<const slr::AudioUnitView> view) {
    postToLvgl([desc, view]() {
        UI::RootWindow::inst()->createUI(desc, view);
    });

    if(PushThread::isRunning()) {
        PushThread::postTask([desc, view]() {
            PushUI::RootWidget::inst()->createUI(desc, view);
        });
    }
}

void removeUI(slr::ID id) {
    postToLvgl([id]() {
        UI::RootWindow::inst()->removeUI(id);
    });
    
    if(PushThread::isRunning()) {
        PushThread::postTask([id]() {
            PushUI::RootWidget::inst()->removeUI(id);
        });
    }
}

void restoreUI(slr::ID id) {
    postToLvgl([id]() {
        UI::RootWindow::inst()->restoreUI(id);
    });

    if(PushThread::isRunning()) {
        PushThread::postTask([id]() {
            PushUI::RootWidget::inst()->restoreUI(id);
        });
    }
}

void deleteUI(slr::ID id) {
    postToLvgl([id]() {
        UI::RootWindow::inst()->deleteUI(id);
    });

    if(PushThread::isRunning()) {
        PushThread::postTask([id]() {
            PushUI::RootWidget::inst()->deleteUI(id);
        });
    }
}


/* Timeline */
//if Time Signature or BPM updated pass true, otherwise false
void updateTimeline(const bool timeSigOrBpm) {
    postToLvgl([timeSigOrBpm]() {
        UI::RootWindow::inst()->updateTimeline(timeSigOrBpm);
    });
}

/* Route Manager */
void updateRouteManager() {
    postToLvgl([]() {
        UI::RootWindow::inst()->_routeManager->liveUpdate();
    });
}

/* Metronome */
void updateMetronomeState(bool onoff) {
    postToLvgl([onoff]() {
        UI::RootWindow::inst()->updateMetronomeState(onoff);
    });
}

void createSequenceUI(const std::shared_ptr<slr::SequenceView> view) {
    postToLvgl([view]() {
        UI::RootWindow::inst()->createSequenceUI(view);
    });

    if(PushThread::isRunning()) {
        //...
    }
}

void createModulationUI(const std::shared_ptr<slr::ModulationPatternView> view) {
    postToLvgl([view]() {
        UI::RootWindow::inst()->createModulationUI(view);
    });

    if(PushThread::isRunning()) {
        //...
    }
}

void updateModulationTargetManager() {
    postToLvgl([]() {
        UI::RootWindow::inst()->_modEngineTargetManagerPopup->update();
    });
}

void clearUI() {
    postToLvgl([]() {
        UI::RootWindow::inst()->clearUI();
    });
    
    if(PushThread::isRunning()) {
        PushThread::postTask([]() {
            PushUI::RootWidget::inst()->clearUI();
        });
    }
}

} //namespace UIControls