/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/Project.h"
#include "inc/ui/uiControls.h"
#include "core/primitives/AudioUnit.h"  
#include "core/ControlEngine.h"
#include "snapshots/ProjectView.h"
#include "Status.h"
#include "logger.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleSetParameterResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Parameter %u updated for unit %u to value %f",
                    resp.setParameter.parameterId, 
                    resp.setParameter.unit->id(),
                    resp.setParameter.value);
                    
        // LOG_WARN("Need somehow update ui...");
        //find unitview
        const slr::ID id = resp.setParameter.unit->id(); 
        AudioUnitView * uview = ctx.projectView->getUnitById(id);
        if(!uview) {
            LOG_ERROR("Failed to find unit with id %u", id);
            return;
        }

        uview->update();
        UIControls::updateModuleUI(id);
    } else {
        LOG_ERROR("Failed to set parameter %u for unit %u to value %f", 
                    resp.setParameter.parameterId, 
                    resp.setParameter.unit->id(),
                    resp.setParameter.value);
    }
}

inline void handleToggleMidiThruResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Midi thru for unit %u is set to %s", 
                    resp.toggleMidiThru.unit->id(),
                    (resp.toggleMidiThru.newState ? "true" : "false"));

        const slr::ID id = resp.toggleMidiThru.unit->id();
        AudioUnitView * view = ctx.projectView->getUnitById(id);
        if(!view) {
            LOG_ERROR("Failed to find unit view with id %u", id);
            return;
        }

        view->update();
        UIControls::updateRouteManager();
    } else {
        LOG_ERROR("Failed to set midi thru for unit %u to %s",
                    resp.toggleMidiThru.unit->id(),
                    (resp.toggleMidiThru.newState ? "true" : "false"));
    }
}

inline void handleToggleOmniHwInputResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Omni hw input for unit %u is set to %s", 
                    resp.toggleOmniHwInput.unit->id(),
                    (resp.toggleOmniHwInput.newState ? "true" : "false"));

        const slr::ID id = resp.toggleOmniHwInput.unit->id();
        AudioUnitView * view = ctx.projectView->getUnitById(id);
        if(!view) {
            LOG_ERROR("Failed to find unit view with id %u", id);
            return;
        }

        view->update();
        UIControls::updateRouteManager();
    } else {
        LOG_ERROR("Failed to set midi thru for unit %u to %s",
                    resp.toggleOmniHwInput.unit->id(),
                    (resp.toggleOmniHwInput.newState ? "true" : "false"));
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::SetParameter &e) {
    LOG_INFO("Set parameter event. Target: %u, parameter: %u, value: %f", 
                    e.targetId, e.parameterId, e.value);
    AudioUnit * unit = ctx.project->getUnitById(e.targetId);
    if(!unit) {
        LOG_ERROR("No target with such id %u", e.targetId);
        return;
    }

    if(!unit->hasParameterWithId(e.parameterId)) {
        LOG_ERROR("No such parameter %u for unit %u", e.parameterId, e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::SetParameter;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.setParameter.unit = unit;
    ctl.setParameter.parameterId = e.parameterId;
    ctl.setParameter.value = e.value;
    ControlEngine::emitRtControl(ctl);
}

inline void handleEvent(const ControlContext &ctx, const Events::ToggleMidiThru &e) {
    LOG_INFO("Toggling midi thru for unit %u new state %s",
                e.targetId, (e.newState ? "true" : "false"));

    AudioUnit *u = ctx.project->getUnitById(e.targetId);
    if(!u) {
        LOG_ERROR("Failed to find unit with id %u", e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ToggleMidiThru;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.toggleMidiThru.unit = u;
    ctl.toggleMidiThru.newState = e.newState;
    ControlEngine::emitRtControl(ctl);
}

inline void handleEvent(const ControlContext &ctx, const Events::ToggleOmniHwInput &e) {
    LOG_INFO("Toggling midi thru for unit %u new state %s",
                e.targetId, (e.newState ? "true" : "false"));

    AudioUnit *u = ctx.project->getUnitById(e.targetId);
    if(!u) {
        LOG_ERROR("Failed to find unit with id %u", e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ToggleOmniHwInput;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.toggleOmniHwInput.unit = u;
    ctl.toggleOmniHwInput.newState = e.newState;
    ControlEngine::emitRtControl(ctl);
}

} //namespace slr
