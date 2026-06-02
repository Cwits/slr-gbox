// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/DeleteUnit.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"
#include "core/primitives/RenderPlan.h"
#include "core/primitives/FileContainer.h"
#include "core/Project.h"
// #include "core/UnitManager.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/FileContainerView.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/push/primitives/UnitUIBase.h"
#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>
#include <memory>
#include <iostream>

namespace slr {
	
DeleteUnitAction::DeleteUnitAction(const ActionBase *base) :
	_action( *(static_cast<const Actions::DeleteUnit*>(base)) ) {
	// _savedId = 0;
}
DeleteUnitAction::~DeleteUnitAction() {
	//need some cleanup, as well as clip containers
	if(_unit) {
		_unit->destroy(_bmanptr);
		UIControls::deleteUI(_action.targetId);
	} //else we done redoing so things are empty
}

void DeleteUnitAction::exec(ControlContext &ctx) {
	assert(getState() == ActionState::Executing);
	
	_bmanptr = ctx.bufferManager;

	switch(_step) {
		case(1): {
			LOG_WARN("Need to save modulations, sequences and everything that is connected to this unit as well");

			UIControls::removeUI(_action.targetId);
			
			if(ctx.project->unitHaveRoutes(_action.targetId)) {
				const std::vector<AudioRoute> &ar = ctx.project->routes();
				for(const AudioRoute &r : ar) {
					if(r._sourceType == AudioRoute::Type::INT && r._sourceId == _action.targetId) {
						_audioRoutes.push_back(r);
					} 
					if(r._targetType == AudioRoute::Type::INT && r._targetId == _action.targetId) {
						_audioRoutes.push_back(r);
					}
				}

				const std::vector<MidiRoute> &mr = ctx.project->midiRoutes();
				for(const MidiRoute &r : mr) {
					if(r._sourceType == MidiRoute::Type::INT && r._sourceId == _action.targetId) {
						_midiRoutes.push_back(r);
					} 
					if(r._targetType == MidiRoute::Type::INT && r._targetId == _action.targetId) {
						_midiRoutes.push_back(r);
					}
				}

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
			
			_unitView = ctx.projectView->removeUnitView(_action.targetId);
			if(!_unitView) {
				abortAction();
				return;
			}
			
			// ClipContainerMap &map = ctx.project->clipContainerMap();
        	// auto it = map.find(_action.targetId);
        	// if(it == map.end()) {
            // 	LOG_ERROR("No clip storage for unit %u exists to delete", _action.targetId);
            // 	abortAction();
            // 	return;
        	// }
        
        	// map.erase(it);
			
        	_unit = ctx.project->removeUnit(_action.targetId);
        	if(_unit == nullptr) {
            	LOG_ERROR("Failed to find unit");
            	abortAction();
            	return;
        	}
        	
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
	ctx.project->appendUnit(std::move(_unit));
	ctx.projectView->appendUnit(_unitView);

	for(auto &ar : _audioRoutes) ctx.project->addRoute(ar);
	for(auto &mr : _midiRoutes) ctx.project->addRoute(mr);

	ctx.projectView->updateRoutes(ctx.project->routes());

	if(!ctx.project->prepareSwappablePlan()) {
        LOG_ERROR("Failed to create new plan");
		abortAction();
		return;
    }

	_flat.project = ctx.project;
	_flat.completed.store(false);
	_task = makeRtTask(&_flat);
				
	ctx.EmitRtTask(&_task);

	UIControls::restoreUI(_action.targetId);

	// auto act = std::make_unique<Actions::CreateNewUnit>();
	// act->name = unitName;
	// act->forcedId = _savedUnit["Unit"][0]["ID"].get<ID>();
	// act->restoredUnit = _savedUnit;
	// EmitAction(std::move(act));
}

void DeleteUnitAction::redo(ControlContext &ctx) {
	/* 
		create new delete action
	*/
	auto act = std::make_unique<Actions::DeleteUnit>();
	act->targetId = _action.targetId;
	EmitAction(std::move(act));
	// _step = 1;
	// _direction = forward;
	// setState(ActionState::Executing);
}

std::unique_ptr<ActionExecutable> createDeleteUnitAction(const ActionBase*base) {
    return std::make_unique<DeleteUnitAction>(base);
}


}