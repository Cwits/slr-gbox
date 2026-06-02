// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/AddNewMidiRoute.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

AddNewMidiRouteAction::AddNewMidiRouteAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::AddNewMidiRoute*>(base)) )
{
}

AddNewMidiRouteAction::~AddNewMidiRouteAction() {

}

void AddNewMidiRouteAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
    		if(!ctx.project->evaluateRoute(_action.route)) {
        		UIControls::floatingWarning("Invalid route");
        		LOG_ERROR("Invalid route");
        		abortAction();
        		return;
    		}
    
    		ctx.project->addRoute(_action.route);
            
            if(_action.swapPlan) {
                if(!ctx.project->prepareSwappablePlan()) {
                    LOG_ERROR("Failed to create swappable plan");
                    abortAction();
                    return;
                }
                
                _flat.project = ctx.project;
                _flat.completed.store(false);
                _task = makeRtTask(&_flat);
                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } else {
                _step = 2;
            }
    	} break; 
    	case(2): {
            ctx.projectView->updateRoutes(ctx.project->midiRoutes());
            UIControls::updateRouteManager();
            
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void AddNewMidiRouteAction::checkWaitingCondition(ControlContext &ctx) {
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

std::unique_ptr<ActionExecutable> createAddNewMidiRouteAction(const ActionBase *base) {
    return std::make_unique<AddNewMidiRouteAction>(base);
}

}
