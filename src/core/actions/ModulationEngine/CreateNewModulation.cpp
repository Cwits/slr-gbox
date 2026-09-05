// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/ModulationEngine/CreateNewModulation.h"
#include "core/actions/ActionBase.h"

#include "core/utility/ControlContext.h"

#include "core/RtEngine.h" //for sample rate
#include "core/drivers/AudioDriver.h" //for sample rate
#include "core/ModulationEngine.h"
#include "core/Project.h"

#include "snapshots/ProjectView.h"
#include "snapshots/ModulationView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

CreateNewModulationAction::CreateNewModulationAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CreateNewModulation*>(base)) )
{

}

CreateNewModulationAction::~CreateNewModulationAction() {

}

void CreateNewModulationAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(_direction == ActionDirection::Forward) {
        switch(_step) {
            case(1): {
                ModulationPattern *ptrn = ctx.project->modulationEngine()->createNewModulationPattern(ctx.engine->driver()->bufferSize(), ctx.engine->driver()->sampleRate());
                if(ptrn == nullptr) { abortAction(); LOG_ERROR("Full"); return; }
                //but actually need to prepare and swap modulation struct in render plan - the same way as with sequences, but this a bit later...
                _createdPtrn = ptrn;

                _flat.engine = ctx.project->modulationEngine();
                _flat.createdPtrn = ptrn;
                _flat.completed.store(false);
                _task = makeRtTask(&_flat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //creatd... 
                std::shared_ptr<ModulationPatternView> modView = ctx.projectView->modulationEngine()->createModulationView(_createdPtrn);
                
                UIControls::createModulationUI(modView);

                setState(ActionState::Finished);
            } break;
        }
    } else if(_direction == ActionDirection::Backward) {
        switch(_step) {
            case(1): {
                _rflat.engine = ctx.project->modulationEngine();
                _rflat.createdPtrn = _createdPtrn;
                _rflat.completed.store(false);
                _task = makeRtTask(&_rflat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //UI will remove itself?

                LOG_WARN("Deleting UI not finished"); //but do it from UI, not here
                setState(ActionState::Finished);
            }
        }
    }
    
}

void CreateNewModulationAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(_direction == ActionDirection::Forward) {
                res = _flat.completed.load(std::memory_order_acquire);
            } else {
                res = _rflat.completed.load(std::memory_order_release);
            }
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}


void CreateNewModulationAction::NewModulation::execRT() {
    engine->playable().push_back(createdPtrn);
    completed.store(true, std::memory_order_release);
}

void CreateNewModulationAction::RemoveMod::execRT() {
    std::vector<ModulationPattern*> &v = engine->playable();

    auto it = std::find_if(
        v.begin(),
        v.end(),
        [ptr = createdPtrn](const ModulationPattern *p) {
            return ptr == p;
        }
    );

    if(it != v.end()) {
        v.erase(it);
    }

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createCreateNewModulationAction(const ActionBase *base) {
    return std::make_unique<CreateNewModulationAction>(base);
}

}