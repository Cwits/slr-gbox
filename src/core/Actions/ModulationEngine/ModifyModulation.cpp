// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/ModulationEngine/ModifyModulation.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/RtEngine.h" //for sample rate
#include "core/drivers/AudioDriver.h" //for sample rate
#include "core/ModulationEngine.h"
#include "core/Project.h"

#include "snapshots/ProjectView.h"
#include "snapshots/ModulationView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ModifyModulationAction::ModifyModulationAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifyModulation*>(base)) )
{

}

ModifyModulationAction::~ModifyModulationAction() {

}

void ModifyModulationAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(_direction == ActionDirection::Forward) {
        switch(_step) {
            case(1): {
                ModulationPattern *ptrn = ctx.project->modulationEngine()->findPatternById(_action.modulationId);
                if(ptrn == nullptr) { 
                    abortAction(); 
                    LOG_ERROR("Full"); 
                    return; 
                }
                //but actually need to prepare and swap modulation struct in render plan - the same way as with sequences, but this a bit later...
                
                _flat.modPtrn = ptrn;

                _toRestore.shape = ptrn->_shape;
                _toRestore.phase = ptrn->_phase;
                _toRestore.rate = ptrn->_lfoRate;
                _toRestore.rateMode = ptrn->_rateMode;
                _toRestore.min = ptrn->_valueMin;
                _toRestore.max = ptrn->_valueMax;

                if(_action.shape) _flat.shape = _action.shape.value();
                else _flat.shape = ptrn->_shape;
                if(_action.phase) _flat.phase = _action.phase.value();
                else _flat.phase = ptrn->_phase;
                if(_action.rate) _flat.rate = _action.rate.value();
                else _flat.rate = ptrn->_lfoRate;
                if(_action.rateMode) _flat.rateMode = _action.rateMode.value();
                else _flat.rateMode = ptrn->_rateMode;
                if(_action.min) _flat.min = _action.min.value();
                else _flat.min = ptrn->_valueMin;
                if(_action.max) _flat.max = _action.max.value();
                else _flat.max = ptrn->_valueMax;

                _flat.completed.store(false);
                _task = makeRtTask(&_flat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //creatd... 
                std::shared_ptr<ModulationPatternView> modView = ctx.projectView->modulationEngine()->findPatternById(_action.modulationId);
                modView->update();
                // UIControls::createModulationUI(modView);

                setState(ActionState::Finished);
            } break;
        }
    } else if(_direction == ActionDirection::Backward) {
        switch(_step) {
            case(1): {
                ModulationPattern *ptrn = ctx.project->modulationEngine()->findPatternById(_action.modulationId);
                if(ptrn == nullptr) { abortAction(); LOG_ERROR("Full"); return; }

                _flat.modPtrn = ptrn;
                _flat.shape = _toRestore.shape.value();
                _flat.phase = _toRestore.phase.value();
                _flat.rate = _toRestore.rate.value();
                _flat.rateMode = _toRestore.rateMode.value();
                _flat.min = _toRestore.min.value();
                _flat.max = _toRestore.max.value();

                _flat.completed.store(false);
                _task = makeRtTask(&_flat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //UI will remove itself?
                
                std::shared_ptr<ModulationPatternView> modView = ctx.projectView->modulationEngine()->findPatternById(_action.modulationId);
                modView->update();

                setState(ActionState::Finished);
            } break;
        }
    }
    
}

void ModifyModulationAction::checkWaitingCondition(ControlContext &ctx) {
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

void ModifyModulationAction::ModifyModulation::execRT() {
    modPtrn->_shape = shape;
    modPtrn->_amplitude = ampitude;
    modPtrn->_phase = phase;
    modPtrn->_lfoRate = rate;
    modPtrn->_rateMode = rateMode;
    modPtrn->_valueMin = min;
    modPtrn->_valueMax = max;
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifyModulationAction(const ActionBase *base) {
    return std::make_unique<ModifyModulationAction>(base);
}

}