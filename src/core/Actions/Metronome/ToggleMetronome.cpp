// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Metronome/ToggleMetronome.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/Metronome.h"
#include "core/utility/helper.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ToggleMetronomeAction::ToggleMetronomeAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ToggleMetronome*>(base)) )
{

}

ToggleMetronomeAction::~ToggleMetronomeAction() {

}

void ToggleMetronomeAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            Metronome *metro = ctx.project->metronome();
            bool oldState = metro->mute();

            _flat.target = metro;
            _flat.parameterId = metro->muteId();
            _flat.value = boolToFloat( (oldState ? false : true) );
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);

            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            bool val = floatToBool(_flat.value);
            UIControls::updateMetronomeState(val ? false : true);

            markDelete();
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ToggleMetronomeAction::checkWaitingCondition() {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

std::unique_ptr<ActionExecutable> createToggleMetronomeAction(const ActionBase *base) {
    return std::make_unique<ToggleMetronomeAction>(base);
}

}