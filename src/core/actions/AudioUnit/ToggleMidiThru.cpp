// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/AudioUnit/ToggleMidiThru.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"
#include "core/primitives/AudioUnit.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

ToggleMidiThruAction::ToggleMidiThruAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ToggleMidiThru*>(base)) )
{

}

ToggleMidiThruAction::~ToggleMidiThruAction() {

}

void ToggleMidiThruAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            AudioUnit *unit = ctx.project->getUnitById(_action.targetId);
            if(!unit) {
                LOG_ERROR("Failed to find unit with id %u", _action.targetId);
                abortAction();
                return;
            }

            _flat.target = unit;
            if(_direction == ActionDirection::Forward) {
                _oldState = unit->isMidiThru();
                _flat.newState = _action.newState;
            } else {
                _flat.newState = _oldState;
            }

            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            LOG_INFO("Midi thru for unit %u is set to %s", 
                        _action.targetId,
                        (_action.newState ? "true" : "false"));

            AudioUnitView * view = ctx.projectView->getUnitById(_action.targetId);
            if(!view) {
                LOG_ERROR("Failed to find unit view with id %u", _action.targetId);
                return;
            }

            if(_direction == ActionDirection::Forward) {
                view->setMidiThru(_action.newState);
            } else {
                view->setMidiThru(_oldState);
            }

            UIControls::updateRouteManager();

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ToggleMidiThruAction::checkWaitingCondition(ControlContext &ctx) {
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


void ToggleMidiThruAction::ToggleThru::execRT() {
    target->setMidiThru(newState);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createToggleMidiThruAction(const ActionBase *base) {
    return std::make_unique<ToggleMidiThruAction>(base);
}

}