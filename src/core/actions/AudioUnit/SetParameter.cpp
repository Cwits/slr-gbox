// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/AudioUnit/SetParameter.h"
#include "core/actions/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"


#include "common/logger.h"

#include <cassert>

namespace slr {

SetParameterAction::SetParameterAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::SetParameter*>(base)) )
{
}

SetParameterAction::~SetParameterAction() {

}

void SetParameterAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);


    switch(_step) {
        case(1): {
            LOG_INFO("Set parameter event targetid: %u, parameterid: %u, value: %f",
                        _action.targetId, _action.parameterId, _action.value);
            
            AudioUnit * target = ctx.project->getUnitById(_action.targetId);
            if(!target) {
                LOG_ERROR("No target with such id %u", _action.targetId);
                abortAction();
                return;
            }

            if(!target->hasParameterWithId(_action.parameterId)) {
                LOG_ERROR("Target don't have parameter with id %u", _action.parameterId);
                abortAction();
                return;
            }

            _flat.completed.store(false);
            _flat.target = target;

            if(_direction == ActionDirection::Forward) {
                _oldValue.targetId = _action.targetId;
                _oldValue.parameterId = _action.parameterId;
                _oldValue.value = target->getParameterRaw(_action.parameterId);
                
                _flat.parameterId = _action.parameterId;
                _flat.value = _action.value;
            } else {
                _flat.parameterId = _oldValue.parameterId;
                _flat.value = _oldValue.value;
            }

            _task = makeRtTask(&_flat);

            setState(ActionState::Waiting);

            ctx.EmitRtTask(&_task);
        } break;
        case(2): {
            //update snapshot
            AudioUnitView *uview = ctx.projectView->getUnitById(_action.targetId);
            if(!uview) { 
                LOG_ERROR("Failed to find unit view for id %u", _action.targetId);
                abortAction();
                return;
            }

            if(_direction == ActionDirection::Forward)
                uview->setParameter(_action.parameterId, _action.value);
            else 
                uview->setParameter(_oldValue.parameterId, _oldValue.value);

            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void SetParameterAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
    if(_step == 1) {
        bool res = _flat.completed.load(std::memory_order_acquire);
        if(res) {
            _step = 2;
            setState(ActionState::Executing);
        }
    } else assert(false && "Unreachable");
}

std::unique_ptr<ActionExecutable> createSetParameterAction(const ActionBase *base) {
    return std::make_unique<SetParameterAction>(base);
}

}
