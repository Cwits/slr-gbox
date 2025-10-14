// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ChangeSignatureBpm

EV
INCLUDE "defines.h"
struct ChangeSigBpm {
    float bpm;
    slr::BarSize sig;
};

FLAT_REQ
struct ChangeSigBpm {
    Timeline * tl; //-> class Timeline;
    float bpm;
    slr::BarSize sig;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::ChangeSigBpm &e) {
    LOG_INFO("Change time signature and bpm. Sig: %u/%u, bpm: %f", e.sig._numerator, e.sig._denominator, e.bpm);
    FlatEvents::FlatControl sig;
    sig.type = FlatEvents::FlatControl::Type::ChangeSigBpm;
    sig.commandId = ControlEngine::generateCommandId();
    sig.changeSigBpm.tl = &ctx.project->timeline();
    sig.changeSigBpm.bpm = e.bpm;
    sig.changeSigBpm.sig = e.sig;
    ControlEngine::emitRtControl(sig);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Timeline.h"
&Timeline::setBpmTimeSig
END_HANDLE

RESP_HANDLE
INCLUDE "core/FlatEvents.h"
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/TimelineView.h"
INCLUDE "logger.h"
void handleChangeSigBpm(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Time signature %d/%d and bpm %f changed",
                    resp.changeSigBpm.sig._numerator,
                    resp.changeSigBpm.sig._denominator,
                    resp.changeSigBpm.bpm);
        ctx.projectView->timeline().update();
        UIControls::updateTimeline(true);
    } else {
        LOG_ERROR("Failed to change time signature %d/%d and bpm %f",
                    resp.changeSigBpm.sig._numerator,
                    resp.changeSigBpm.sig._denominator,
                    resp.changeSigBpm.bpm);
    }
}
END_HANDLE

END_BLOCK