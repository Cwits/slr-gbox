// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK LoopPosition

EV
INCLUDE "defines.h"
struct LoopPosition {
    frame_t start;
    frame_t end;
};

FLAT_REQ
INCLUDE "defines.h"
struct LoopPosition {
    Timeline * tl; //-> class Timeline;
    frame_t start;
    frame_t end;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::LoopPosition &e) {
    LOG_INFO("Changing loop positions start %lu end %lu", e.start, e.end);

    FlatEvents::FlatControl loop;
    loop.type = FlatEvents::FlatControl::Type::LoopPosition;
    loop.loopPosition.tl = &ctx.project->timeline();
    loop.loopPosition.start = e.start;
    loop.loopPosition.end = e.end;
    ControlEngine::emitRtControl(loop);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Timeline.h"
&Timeline::setLoopPosition
END_HANDLE

RESP_HANDLE
INCLUDE "Status.h"
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/TimelineView.h"
INCLUDE "logger.h"
void handleLoopPositionChange(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Loop position changed to start %lu, end %lu", resp.loopPosition.start, resp.loopPosition.end);
        ctx.projectView->timeline().update();
        UIControls::updateTimeline(false);
    } else {
        LOG_ERROR("Failed to change loop position start %lu, end %lu", resp.loopPosition.start, resp.loopPosition.end);
    }
}
END_HANDLE

END_BLOCK