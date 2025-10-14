// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK RequestPlayhead

EV
struct RequestPlayhead {
};

FLAT_REQ
struct RequestPlayhead {
    Timeline * timeline; //-> class Timeline;
};

FLAT_RESP
INCLUDE "defines.h"
struct RequestPlayhead {
    frame_t position;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/Project.h"
void handleEvent(const ControlContext &ctx, const Events::RequestPlayhead &e) {
    FlatEvents::FlatControl upd;
    upd.type = FlatEvents::FlatControl::Type::RequestPlayhead;
    upd.requestPlayhead.timeline = &ctx.project->timeline();
    ControlEngine::emitRtControl(upd);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Timeline.h"
&Timeline::requestPlayhead
END_HANDLE

RESP_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "ui/uiControls.h"
void handleRequestPlayhead(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    // ProjectView * psnap = getEditSnapshot();
	// // psnap->setPlayhead(resp.playhead.pos);
	// psnap->timeline().setPlayhead(resp.playhead.position);
	// swapSnapshot();
    ctx.projectView->timeline().setPlayhead(resp.requestPlayhead.position);
    UIControls::updatePlayheadPosition(resp.requestPlayhead.position);
}
END_HANDLE

END_BLOCK