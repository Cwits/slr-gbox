// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/StepSequencer/ModifySequence.h"

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

ModifySequenceAction::ModifySequenceAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifySequence*>(base)) )
{
}

ModifySequenceAction::~ModifySequenceAction() {

}

void ModifySequenceAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            Sequence * editable = ctx.project->stepSequencer()->findSequenceById(_action.sequenceId);
            if(!editable) {
                LOG_ERROR("Failed to find sequence %lu", _action.sequenceId);
                abortAction();
                return;
            }
            
            uint32_t stepCount;
            if(_action.stepCount) {
                stepCount = _action.stepCount.value();
                if(stepCount > EVENTS_COUNT) {
                    LOG_ERROR("Too much steps for sequence");
                    abortAction();
                    return;
                }
            } else {
                stepCount = editable->_stepCount;
            }

            StepDuration dur;
            if(_action.duration) dur = _action.duration.value();
            else dur = editable->stepDuration();

            _flatModify.tl = &ctx.project->timeline();
            _flatModify.stepCount = stepCount;
            _flatModify.duration = dur;
            _flatModify.editable = editable;
            _flatModify.completed.store(false, std::memory_order_relaxed);

            _task = makeRtTask(&_flatModify);
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

void ModifySequenceAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flatModify.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ModifySequenceAction::Modify::execRT() {
    if(editable->stepDuration() != duration) {
        // editable->recalculateEventPositions(*tl, duration);
        editable->setDuration(duration);
    }

    if(editable->_stepCount != stepCount) {
        editable->_stepCount = stepCount;
    }

    completed.store(true, std::memory_order_release);

}

std::unique_ptr<ActionExecutable> createModifySequenceAction(const ActionBase *base) {
    return std::make_unique<ModifySequenceAction>(base);
}

}