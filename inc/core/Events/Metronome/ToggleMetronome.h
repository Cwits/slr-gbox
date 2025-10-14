// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ToggleMetronome

EV
struct ToggleMetronome {
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/Metronome.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/utility/helper.h"
void handleEvent(const ControlContext &ctx, const Events::ToggleMetronome &e) {
    LOG_INFO("Toggle mentronome event");

    Metronome *metro = ctx.project->metronome();
    bool oldState = metro->mute();

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::SetParameter;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.setParameter.unit = ctx.project->metronome();
    ctl.setParameter.parameterId = metro->muteId();
    ctl.setParameter.value = boolToFloat((oldState ? false : true));

    ControlEngine::awaitRtResult(ctl, [](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status == Status::Ok) {
            bool val = floatToBool(resp.setParameter.value); //it's mute parameter - 0 means unmute, 1 means mute
            LOG_INFO("Toggle metronome new state: %s", (val ? "Off" : "On"));

            //update ui
            UIControls::updateMetronomeState(val ? false : true);
        } else {
            LOG_ERROR("Failed to set metronome to: %s", (floatToBool(resp.setParameter.value) ? "Off" : "On"));
        }
    });
}
END_HANDLE

END_BLOCK