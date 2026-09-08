// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/StepSequencer/CreateNewSequence.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"
#include "core/StepSequencer.h"
#include "core/RenderPlan.h"

#include "snapshots/ProjectView.h"
#include "snapshots/SequenceView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

CreateNewSequenceAction::CreateNewSequenceAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CreateNewSequence*>(base)) )
{

}

CreateNewSequenceAction::~CreateNewSequenceAction() {

}

void CreateNewSequenceAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(_direction == ActionDirection::Forward) {
        switch(_step) {
            case(1): {
                Sequence * seq = ctx.project->stepSequencer()->createNewSequence();
                seq->initDefault(ctx.project->timeline());

                _createdPtr = seq;
                
                const RenderPlan * plan = ctx.project->getSwappablePlan(ctx, (uint16_t)PlanBuilder::PlanRebuild::All);
                if(!plan) {
                    LOG_ERROR("Failed to rebuild Render Plan");
                    abortAction();
                    return;
                }

                _flatSwap.plan = plan;
                _flatSwap.engine = ctx.engine;
                _flatSwap.completed.store(false);

                _task = makeRtTask(&_flatSwap);
                
                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //created
                ctx.project->swapPlan();
                std::shared_ptr<SequenceView> seqv = ctx.projectView->stepSequencer()->createSequenceView(_createdPtr);

                UIControls::createSequenceUI(seqv);

                setState(ActionState::Finished);
            } break;
            default: assert(false && "Unreachable"); break;
        }
    } else {
        switch(_step) {
            case(1): {
                LOG_WARN("Not working yet");
                abortAction();
                return;
                _rflat.engine = ctx.project->stepSequencer();
                _rflat.tofind = _createdPtr;
                _rflat.completed.store(false);
                _task = makeRtTask(&_rflat);
                
                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } break;
            case(2): {
                //created
                //UI will remove itself
                LOG_WARN("Deleting UI not finished"); //but delete it from UI, not from here
                // _rflat.extracted.release();
                setState(ActionState::Finished);
            } break;
            default: assert(false && "Unreachable"); break;
        }
    }
}

void CreateNewSequenceAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);


    switch(_step) {
        case(1): {
            bool res = false; //_flat.completed.load(std::memory_order_acquire);
            if(_direction == ActionDirection::Forward) {
                res = _flat.completed.load(std::memory_order_acquire);
            } else {
                res = _rflat.completed.load(std::memory_order_acquire);
            }
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}


void CreateNewSequenceAction::AddFlat::execRT() {
    // engine->playable().push_back(newSeq);
    completed.store(true, std::memory_order_release);
}

void CreateNewSequenceAction::RemoveFlat::execRT() {
    // std::vector<Sequence*> & vec = engine->playable();
    
    // auto it = std::find_if(vec.begin(), vec.end(), [ptr = tofind](const Sequence * s) {
    //     return ptr == s;
    // });

    // if(it != vec.end()) {
    //     vec.erase(it);
    // }

    // completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createCreateNewSequenceAction(const ActionBase *base) {
    return std::make_unique<CreateNewSequenceAction>(base);
}

}