// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ToggleOmniHwInput

EV
INCLUDE "defines.h"
struct ToggleOmniHwInput {
    ID targetId;
    bool newState;
};

FLAT_REQ
struct ToggleOmniHwInput {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::ToggleOmniHwInput &e) {
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
END_HANDLE

RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::toggleOmniHwInput
END_HANDLE

RESP_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "inc/ui/uiControls.h"
INCLUDE "logger.h"
INCLUDE "Status.h"
void handleToggleOmniHwInputResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
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
END_HANDLE

END_BLOCK