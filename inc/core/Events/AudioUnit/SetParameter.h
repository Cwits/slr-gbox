// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SetParameter

EV
INCLUDE "defines.h"
struct SetParameter {
    ID targetId;
    ID parameterId;
    float value;
};

FLAT_REQ
struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"  
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::SetParameter &e) {
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
END_HANDLE

RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::setParameter
END_HANDLE

RESP_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "logger.h"
INCLUDE "Status.h"
void handleSetParameterResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
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
END_HANDLE

END_BLOCK