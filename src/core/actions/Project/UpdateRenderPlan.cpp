// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/Project/UpdateRenderPlan.h"
#include "core/actions/ActionBase.h"

#include "core/utility/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

UpdateRenderPlanAction::UpdateRenderPlanAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::UpdateRenderPlan*>(base)) )
{

}

UpdateRenderPlanAction::~UpdateRenderPlanAction() {

}

void UpdateRenderPlanAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            // if(!ctx.project->prepareSwappablePlan()) {
            //     LOG_ERROR("Failed to create swappable plan");
            //     abortAction();
            //     return;
            // }
            
            // _flat.project = ctx.project;
            const RenderPlan * plan = ctx.project->getSwappablePlan(ctx, (uint16_t)PlanBuilder::PlanRebuild::All);
            if(!plan) {
                LOG_ERROR("Failed to rebuild plan");
                abortAction();
                return;
            }

            _flat.plan = plan;
            _flat.engine = ctx.engine;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {

             
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void UpdateRenderPlanAction::checkWaitingCondition(ControlContext &ctx) {
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

std::unique_ptr<ActionExecutable> createUpdateRenderPlanAction(const ActionBase *base) {
    return std::make_unique<UpdateRenderPlanAction>(base);
}

}