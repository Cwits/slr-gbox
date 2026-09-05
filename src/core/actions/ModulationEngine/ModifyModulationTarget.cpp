// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/ModulationEngine/ModifyModulationTarget.h"
#include "core/actions/ActionBase.h"

#include "core/utility/ControlContext.h"

#include "core/ModulationEngine.h"
#include "core/Project.h"
#include "core/primitives/Parameter.h"
#include "core/primitives/ParameterArray.h"
#include "core/primitives/AudioUnit.h"


#include "snapshots/ProjectView.h"
#include "snapshots/ModulationView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

ModifyModulationTargetAction::ModifyModulationTargetAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifyModulationTarget*>(base)) )
{
}

ModifyModulationTargetAction::~ModifyModulationTargetAction() {

}

void ModifyModulationTargetAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(_direction == ActionDirection::Forward) {
        switch(_step) {
            case(1): {
                ModulationPattern *ptrn = ctx.project->modulationEngine()->findPatternById(_action.modulationID);
                if(ptrn == nullptr) { 
                    abortAction(); 
                    LOG_ERROR("Full"); 
                    return; 
                }
                
                ModTargetArray arr = ptrn->_targets;
                _toRestore = arr;

                if(_action.addOrRemove) {
                    //add target
                    if(!arr.canAddTarget()) {
                        abortAction();
                        LOG_WARN("No more targets can be added to mod %lu", _action.modulationID);
                        return;
                    }
                    arr.addTarget(_action.type, _action.targetID, _action.parameterID);

                    //prepare 
                    switch(_action.type) {
                        case(ModulationTargetType::modulation): ; break;
                        case(ModulationTargetType::stepSequence): ; break;
                        case(ModulationTargetType::unit): {
                            AudioUnit * u = ctx.project->getUnitById(_action.targetID);
                            if(!u) {
                                abortAction();
                                LOG_ERROR("No such unit");
                                return;
                            }

                            if(!u->hasParameterWithId(_action.parameterID)) {
                                abortAction();
                                LOG_ERROR("Unit %lu don't have parameter %lu", _action.targetID, _action.parameterID);
                                return;
                            }

                            _flat.base = u->getParameterByID(_action.parameterID);
                            _flat.addOrRemove = true;
                            _flat.ptr = ptrn->_data;
                            _flat.ammount = 100;
                        } break;
                    }
                } else {
                    //remove target
                    if(!arr.haveTargets()) {
                        abortAction();
                        LOG_ERROR("Can't remove things that is not there lol");
                        return;
                    }
                    arr.removeTarget(_action.type, _action.targetID, _action.parameterID);
                    
                    switch(_action.type) {
                        case(ModulationTargetType::modulation): ; break;
                        case(ModulationTargetType::stepSequence): ; break;
                        case(ModulationTargetType::unit): {
                            AudioUnit * u = ctx.project->getUnitById(_action.targetID);
                            if(!u) {
                                abortAction();
                                LOG_ERROR("No such unit");
                                return;
                            }

                            if(!u->hasParameterWithId(_action.parameterID)) {
                                abortAction();
                                LOG_ERROR("Unit %lu don't have parameter %lu", _action.targetID, _action.parameterID);
                                return;
                            }

                            _flat.base = u->getParameterByID(_action.parameterID);
                            _flat.addOrRemove = false;
                            _flat.ptr = ptrn->_data;
                            _flat.ammount = 0;
                        } break;
                    }
                }

                // _flat.modPtrn = ptrn;
                // _flat.arr = arr;
                ptrn->_targets = arr;
                
                _flat.completed.store(false);
                _task = makeRtTask(&_flat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                

                std::shared_ptr<ModulationPatternView> modView = ctx.projectView->modulationEngine()->findPatternById(_action.modulationID);
                modView->update();
                // UIControls::createModulationUI(modView);
                UIControls::updateModulationTargetManager();

                setState(ActionState::Finished);
            } break;
        }
    } else if(_direction == ActionDirection::Backward) {
        switch(_step) {
            case(1): {
                // ModulationPattern *ptrn = ctx.project->modulationEngine()->findPatternById(_action.modulationId);
                // if(ptrn == nullptr) { abortAction(); LOG_ERROR("Full"); return; }

                // _flat.modPtrn = ptrn;
                // _flat.shape = _toRestore.shape.value();
                // _flat.phase = _toRestore.phase.value();
                // _flat.rate = _toRestore.rate.value();
                // _flat.rateMode = _toRestore.rateMode.value();
                // _flat.min = _toRestore.min.value();
                // _flat.max = _toRestore.max.value();

                // _flat.completed.store(false);
                // _task = makeRtTask(&_flat);

                // setState(ActionState::Waiting);
                // ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //UI will remove itself?
                
                // std::shared_ptr<ModulationPatternView> modView = ctx.projectView->modulationEngine()->findPatternById(_action.modulationId);
                // modView->update();

                // setState(ActionState::Finished);
            } break;
        }
    }
    
}

void ModifyModulationTargetAction::checkWaitingCondition(ControlContext &ctx) {
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

void ModifyModulationTargetAction::ModifyModulation::execRT() {
    if(addOrRemove) base->appendModulation(ptr, ammount);
    else base->removeModulation(ptr);

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifyModulationTargetAction(const ActionBase *base) {
    return std::make_unique<ModifyModulationTargetAction>(base);
}

}