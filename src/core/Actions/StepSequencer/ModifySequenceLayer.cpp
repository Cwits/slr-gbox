// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/StepSequencer/ModifySequenceLayer.h"

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

ModifySequenceLayerAction::ModifySequenceLayerAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifySequenceLayer*>(base)) )
{
}

ModifySequenceLayerAction::~ModifySequenceLayerAction() {

}

void ModifySequenceLayerAction::exec(ControlContext &ctx) {
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
            
            Sequence::Layer &l = editable->_layers[_action.layer];

            int note = 0;
            if(_action.note) {
                note = _action.note.value();
                if(note > 127) {
                    LOG_ERROR("Note too big %i", note);
                    abortAction();
                    return;
                }
            } else {
                note = l._note;
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
                velocity = l._velocity;
            }

            bool mute = false;
            if(_action.mute) mute = _action.mute.value();
            else mute = l._mute;

            bool active = false;
            if(_action.active) active = _action.active.value();
            else active = l._active;

            const AudioUnit * target = nullptr;
            if(_action.targetId) {
                target = ctx.project->getUnitById(_action.targetId.value());
                if(!target) {
                    LOG_ERROR("Failed to find unit with id %lu", _action.targetId.value());
                    abortAction();
                    return;
                }
            } else {
                target = l._target;
            }

            _flatModLayer.editable = editable;
            _flatModLayer.layer = _action.layer;
            _flatModLayer.note = static_cast<uint8_t>(note);
            _flatModLayer.velocity = static_cast<uint8_t>(velocity);
            _flatModLayer.mute = mute;
            _flatModLayer.active = active;
            _flatModLayer.target = target;

            _flatModLayer.completed.store(false, std::memory_order_relaxed);

            _task = makeRtTask(&_flatModLayer);
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

void ModifySequenceLayerAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flatModLayer.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ModifySequenceLayerAction::ModifyLayer::execRT(/*const AudioContext &ctx*/) {
    Sequence::Layer &l = editable->_layers[layer];
    l._note = note;
    l._velocity = velocity;
    l._mute = mute;
    l._active = active;
    l._target = target;

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifySequenceLayerAction(const ActionBase *base) {
    return std::make_unique<ModifySequenceLayerAction>(base);
}

}