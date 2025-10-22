/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/utility/helper.h"
#include "core/Project.h"
#include "core/FlatEvents.h"
#include "core/Metronome.h"
#include "core/ControlEngine.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleEvent(const ControlContext &ctx, const Events::ToggleMetronome &e) {
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

} //namespace slr
