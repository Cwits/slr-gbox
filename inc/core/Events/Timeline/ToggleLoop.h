// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ToggleLoop

EV
struct ToggleLoop {
    bool newState;
};

FLAT_REQ
struct ToggleLoop {
    Timeline * timeline; //-> class Timeline;
    bool newState;
};

EV_HANDLE
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::ToggleLoop &e) {
    LOG_INFO("Loop event triggered, new expected loop state: %s", (e.newState ? "On" : "Off"));
    FlatEvents::FlatControl loop;
    loop.type = FlatEvents::FlatControl::Type::ToggleLoop;
    loop.toggleLoop.timeline = &ctx.project->timeline();
    loop.toggleLoop.newState = e.newState;
    ControlEngine::emitRtControl(loop);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Timeline.h"
&Timeline::toggleLoop
END_HANDLE

RESP_HANDLE
INCLUDE "core/FlatEvents.h"
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/TimelineView.h"
INCLUDE "logger.h"
void handleToggleLoop(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Loop event response, new loop state: %s", (resp.toggleLoop.newState ? "On" : "Off"));
        ctx.projectView->timeline().update();
        UIControls::updateTimeline(false);
    } else {
        LOG_ERROR("Failed to set loop state to %s", (resp.toggleLoop.newState ? "On" : "Off"));
    }
}
END_HANDLE

END_BLOCK