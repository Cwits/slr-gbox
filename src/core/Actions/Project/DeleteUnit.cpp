// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/DeleteUnit.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"
#include "core/primitives/RenderPlan.h"
#include "core/primitives/FileContainer.h"
#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>
#include <memory>

namespace slr {
	
DeleteUnitAction::DeleteUnitAction(const ActionBase *base) :
	_action( *(static_cast<const Actions::DeleteUnit*>(base)) ) {
}
DeleteUnitAction::~DeleteUnitAction() {}

void DeleteUnitAction::exec(ControlContext &ctx) {
	assert(getState() == ActionState::Executing);
	
	switch(_step) {
		case(1): {
			/* 
				serialize everything related to this targetId(routes, paths to clips, parameters and etc...) to some blob or smth... 
				only than delete
			*/
			UIControls::destroyUnitUI(_action.targetId);
			
			
			if(ctx.project->unitHaveRoutes(_action.targetId)) {
				ctx.project->removeRoutesForId(_action.targetId);
				ctx.projectView->updateRoutes(ctx.project->routes());
				
                if(!ctx.project->prepareSwappablePlan()) {
                    LOG_ERROR("Failed to create new plan");
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
			bool res = true;
			
			std::shared_ptr<AudioUnitView> unitview = ctx.projectView->removeUnitView(_action.targetId);
			if(!unitview) {
				abortAction();
				return;
			}
			
			ClipContainerMap &map = ctx.project->clipContainerMap();
        	auto it = map.find(_action.targetId);
        	if(it == map.end()) {
            	LOG_ERROR("No clip storage for unit %u exists to delete", _action.targetId);
            	abortAction();
            	return;
        	}
        
        	map.erase(it);
			
        	std::unique_ptr<AudioUnit> unit = ctx.project->removeUnit(_action.targetId);
        	if(unit == nullptr) {
            	LOG_ERROR("Failed to find unit");
            	abortAction();
            	return;
        	}

        	unit->destroy(ctx.bufferManager);
        	
        	markDelete();
        	setState(ActionState::Finished);
		} break;
        default: assert(false && "Unreachable"); break;
	}
}
void DeleteUnitAction::checkWaitingCondition(ControlContext &ctx) {
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

void DeleteUnitAction::undo(ControlContext &ctx) {
	//huh??
	/*
		restore unit (with original id) from serialized blob
	*/
}

void DeleteUnitAction::redo(ControlContext &ctx) {
	/* 
		create new delete action
	*/
}

std::unique_ptr<ActionExecutable> createDeleteUnitAction(const ActionBase*base) {
    return std::make_unique<DeleteUnitAction>(base);
}


}