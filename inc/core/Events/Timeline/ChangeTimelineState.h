// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ChangeTimelineState

EV
INCLUDE "defines.h"
struct ChangeTimelineState {
    TimelineState state;
};

FLAT_REQ
INCLUDE "defines.h"
struct ChangeTimelineState {
    TimelineState state;
    Timeline * timeline; //-> class Timeline;
};

FLAT_RESP
INCLUDE "defines.h"
struct ChangeTimelineState {
    TimelineState state;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/Project.h"
INCLUDE "defines.h"
void handleEvent(const ControlContext &ctx, const Events::ChangeTimelineState &e) {
    FlatEvents::FlatControl tlstate;
    tlstate.type = FlatEvents::FlatControl::Type::ChangeTimelineState;
    tlstate.commandId = ControlEngine::generateCommandId();
    tlstate.changeTimelineState.timeline = &ctx.project->timeline();
    tlstate.changeTimelineState.state = e.state;
    ControlEngine::emitRtControl(tlstate);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Timeline.h"
&Timeline::changeTimelineState
END_HANDLE

RESP_HANDLE
INCLUDE "defines.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "core/Timeline.h"
INCLUDE "logger.h"
void handleRequestChangeTimelineState(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::NotOk) { 
        LOG_ERROR("Failed to change timeline state");
    }

    ctx.projectView->timeline().update();
    
    UIControls::updateTimeline(false);
}
END_HANDLE

END_BLOCK