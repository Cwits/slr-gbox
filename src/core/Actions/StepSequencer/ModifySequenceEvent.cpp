// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/StepSequencer/ModifySequenceEvent.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/primitives/AudioUnit.h"
#include "core/Project.h"
#include "core/StepSequencer.h"

#include "snapshots/ProjectView.h"
#include "snapshots/SequenceView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ModifySequenceEventAction::ModifySequenceEventAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifySequenceEvent*>(base)) )
{
}

ModifySequenceEventAction::~ModifySequenceEventAction() {

}

void ModifySequenceEventAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            Sequence * editable = ctx.project->stepSequencer()->findSequenceById(_action.sequenceId);
            if(!editable) {
                LOG_ERROR("Failed to find sequence %lu", _action.sequenceId);
                abortAction();
                return;
            }
            
            if(_action.layer > LAYERS_COUNT) {
                LOG_ERROR("Too big layer num");
                abortAction();
                return;
            }

            if(_action.eventNum > EVENTS_COUNT ||
                _action.eventNum > editable->_stepCount) {
                LOG_ERROR("Too big event num");
                abortAction();
                return;
            }

            Sequence::Layer &l = editable->_layers[_action.layer];

            StepEvent &ev = l._events[_action.eventNum];

            int note = 0;
            if(_action.note) {
                note = _action.note.value();
                if(note > 127) {
                    LOG_ERROR("Note too big %i", note);
                    abortAction();
                    return;
                }
            } else {
                note = ev._event.note;
            }
            
            int velocity = 0;
            if(_action.velocity) {
                velocity = _action.velocity.value();
                if(velocity > 127) {
                    LOG_ERROR("Velocity too big %i", velocity);
                    abortAction();
                    return;
                }
            } else {
                velocity = ev._event.velocity;
            }

            bool enabled = false;
            if(_action.enabled) enabled = _action.enabled.value();
            else enabled = ev._enabled;


            _flatModEvent.editable = editable;
            _flatModEvent.layer = _action.layer;
            _flatModEvent.eventNum = _action.eventNum;
            _flatModEvent.enabled = enabled;
            _flatModEvent.note = static_cast<uint8_t>(note);
            _flatModEvent.velocity = static_cast<uint8_t>(velocity);
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

void ModifySequenceEventAction::checkWaitingCondition(ControlContext &ctx) {
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

void ModifySequenceEventAction::ModifyEvent::execRT(/*const AudioContext &ctx*/) {
    StepEvent &ev = editable->_layers[layer]._events[eventNum];
    ev._enabled = enabled;
    ev._event.note = note;
    ev._event.velocity = velocity;

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifySequenceEventAction(const ActionBase *base) {
    return std::make_unique<ModifySequenceEventAction>(base);
}

}