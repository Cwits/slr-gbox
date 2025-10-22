/* This file is generated automatically, do not edit manually */
#pragma once
#include "snapshots/ProjectView.h"
#include "snapshots/TimelineView.h"
#include "core/Project.h"
#include "core/FlatEvents.h"
#include "defines.h"
#include "ui/uiControls.h"
#include "core/Timeline.h"
#include "core/ControlEngine.h"
#include "Status.h"
#include "logger.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleRequestPlayhead(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    // ProjectView * psnap = getEditSnapshot();
	// // psnap->setPlayhead(resp.playhead.pos);
	// psnap->timeline().setPlayhead(resp.playhead.position);
	// swapSnapshot();
    ctx.projectView->timeline().setPlayhead(resp.requestPlayhead.position);
    UIControls::updatePlayheadPosition(resp.requestPlayhead.position);
}

inline void handleRequestChangeTimelineState(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::NotOk) { 
        LOG_ERROR("Failed to change timeline state");
    }

    ctx.projectView->timeline().update();
    
    UIControls::updateTimeline(false);
}

inline void handleToggleLoop(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Loop event response, new loop state: %s", (resp.toggleLoop.newState ? "On" : "Off"));
        ctx.projectView->timeline().update();
        UIControls::updateTimeline(false);
    } else {
        LOG_ERROR("Failed to set loop state to %s", (resp.toggleLoop.newState ? "On" : "Off"));
    }
}

inline void handleChangeSigBpm(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
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

inline void handleLoopPositionChange(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Loop position changed to start %lu, end %lu", resp.loopPosition.start, resp.loopPosition.end);
        ctx.projectView->timeline().update();
        UIControls::updateTimeline(false);
    } else {
        LOG_ERROR("Failed to change loop position start %lu, end %lu", resp.loopPosition.start, resp.loopPosition.end);
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::RequestPlayhead &e) {
    FlatEvents::FlatControl upd;
    upd.type = FlatEvents::FlatControl::Type::RequestPlayhead;
    upd.requestPlayhead.timeline = &ctx.project->timeline();
    ControlEngine::emitRtControl(upd);
}

inline void handleEvent(const ControlContext &ctx, const Events::ChangeTimelineState &e) {
    FlatEvents::FlatControl tlstate;
    tlstate.type = FlatEvents::FlatControl::Type::ChangeTimelineState;
    tlstate.commandId = ControlEngine::generateCommandId();
    tlstate.changeTimelineState.timeline = &ctx.project->timeline();
    tlstate.changeTimelineState.state = e.state;
    ControlEngine::emitRtControl(tlstate);
}

inline void handleEvent(const ControlContext &ctx, const Events::ToggleLoop &e) {
    LOG_INFO("Loop event triggered, new expected loop state: %s", (e.newState ? "On" : "Off"));
    FlatEvents::FlatControl loop;
    loop.type = FlatEvents::FlatControl::Type::ToggleLoop;
    loop.toggleLoop.timeline = &ctx.project->timeline();
    loop.toggleLoop.newState = e.newState;
    ControlEngine::emitRtControl(loop);
}

inline void handleEvent(const ControlContext &ctx, const Events::ChangeSigBpm &e) {
    LOG_INFO("Change time signature and bpm. Sig: %u/%u, bpm: %f", e.sig._numerator, e.sig._denominator, e.bpm);
    FlatEvents::FlatControl sig;
    sig.type = FlatEvents::FlatControl::Type::ChangeSigBpm;
    sig.commandId = ControlEngine::generateCommandId();
    sig.changeSigBpm.tl = &ctx.project->timeline();
    sig.changeSigBpm.bpm = e.bpm;
    sig.changeSigBpm.sig = e.sig;
    ControlEngine::emitRtControl(sig);
}

inline void handleEvent(const ControlContext &ctx, const Events::LoopPosition &e) {
    LOG_INFO("Changing loop positions start %lu end %lu", e.start, e.end);

    FlatEvents::FlatControl loop;
    loop.type = FlatEvents::FlatControl::Type::LoopPosition;
    loop.loopPosition.tl = &ctx.project->timeline();
    loop.loopPosition.start = e.start;
    loop.loopPosition.end = e.end;
    ControlEngine::emitRtControl(loop);
}

} //namespace slr
