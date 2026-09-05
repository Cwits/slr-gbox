// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/StepSequencer/ModifySequenceTarget.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/primitives/AudioUnit.h"
#include "core/Project.h"
#include "core/StepSequencer.h"

#include "snapshots/ProjectView.h"
#include "snapshots/SequenceView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

ModifySequenceTargetAction::ModifySequenceTargetAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifySequenceTarget*>(base)) )
{
}

ModifySequenceTargetAction::~ModifySequenceTargetAction() {

}

void ModifySequenceTargetAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            Sequence * editable = ctx.project->stepSequencer()->findSequenceById(_action.sequenceId);
            if(!editable) {
                LOG_ERROR("Failed to find sequence %lu", _action.sequenceId);
                abortAction();
                return;
            }
            
            const AudioUnit * unit = ctx.project->getUnitById(_action.targetId);
            if(!unit) {
                LOG_ERROR("No such unit with id  %lu", _action.targetId);
                abortAction();
                return;
            }

            _flatModEvent.editable = editable;
            _flatModEvent.addTarget = _action.addTarget;
            _flatModEvent.target = unit;
            _flatModEvent.completed.store(false, std::memory_order_relaxed);
            
            _task = makeRtTask(&_flatModEvent);
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            std::shared_ptr<slr::SequenceView> sview = ctx.projectView->stepSequencer()->findSequenceById(_action.sequenceId);
            sview->update(true);
            // //ui pulls by himself

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ModifySequenceTargetAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flatModEvent.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ModifySequenceTargetAction::ModifyEvent::execRT(/*const AudioContext &ctx*/) {
    std::array<const AudioUnit*, TARGET_COUNT> &arr = editable->_targets;
    if(addTarget) {

        bool done = false;
        for(int i=0; i<TARGET_COUNT; ++i) {
            if(arr[i] == nullptr) {
                arr[i] = target;
                done = true;
                break;
            }
        }

        if(!done) {
            ///idk...
            arr[TARGET_COUNT-1] = target;
        }
    } else {
        for(int i=0; i<TARGET_COUNT; ++i) {
            if(arr[i] == target) {
                arr[i] = nullptr;
                break;
            }
        }
    }

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifySequenceTargetAction(const ActionBase *base) {
    return std::make_unique<ModifySequenceTargetAction>(base);
}

}