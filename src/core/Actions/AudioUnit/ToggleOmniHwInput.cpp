// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/AudioUnit/ToggleOmniHwInput.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/primitives/AudioUnit.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ToggleOmniHwAction::ToggleOmniHwAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ToggleOmniHwInput*>(base)) )
{

}

ToggleOmniHwAction::~ToggleOmniHwAction() {

}

void ToggleOmniHwAction::exec(ControlContext &ctx) {
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
                _flat.newState = _action.newState;
                _oldState = unit->isOmniHwInput();
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
                view->setOmniHw(_action.newState);
            } else if(_direction == ActionDirection::Backward) {
                view->setOmniHw(_oldState);
            }

            UIControls::updateRouteManager();

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ToggleOmniHwAction::checkWaitingCondition(ControlContext &ctx) {
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


void ToggleOmniHwAction::ToggleOmniHw::execRT() {
    target->setOmniHw(newState);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createToggleOmniHwAction(const ActionBase *base) {
    return std::make_unique<ToggleOmniHwAction>(base);
}

}