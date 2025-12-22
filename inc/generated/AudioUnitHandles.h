/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/FlatEvents.h"
#include "snapshots/ProjectView.h"
#include "snapshots/FileContainerView.h"
#include "core/FileWorker.h"
#include "core/FileTasks.h"
#include "inc/ui/uiControls.h"
#include <string>
#include "logger.h"  
#include "Status.h"
#include <memory>
#include "core/primitives/AudioUnit.h"  
#include "ui/uiControls.h"
#include "logger.h"
#include "modules/Track/TrackView.h"
#include "snapshots/AudioUnitView.h"
#include "core/primitives/FileContainer.h"
#include "core/primitives/AudioUnit.h"
#include "core/ControlEngine.h"
#include "core/Project.h"
#include "core/primitives/File.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleSetParameterResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Parameter %u updated for unit %u to value %f",
                    resp.setParameter.parameterId, 
                    resp.setParameter.unit->id(),
                    resp.setParameter.value);
                    
        // LOG_WARN("Need somehow update ui...");
        //find unitview
        const slr::ID id = resp.setParameter.unit->id(); 
        AudioUnitView * uview = ctx.projectView->getUnitById(id);
        if(!uview) {
            LOG_ERROR("Failed to find unit with id %u", id);
            return;
        }

        uview->update();
        UIControls::updateModuleUI(id);
    } else {
        LOG_ERROR("Failed to set parameter %u for unit %u to value %f", 
                    resp.setParameter.parameterId, 
                    resp.setParameter.unit->id(),
                    resp.setParameter.value);
    }
}

inline void handleToggleMidiThruResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Midi thru for unit %u is set to %s", 
                    resp.toggleMidiThru.unit->id(),
                    (resp.toggleMidiThru.newState ? "true" : "false"));

        const slr::ID id = resp.toggleMidiThru.unit->id();
        AudioUnitView * view = ctx.projectView->getUnitById(id);
        if(!view) {
            LOG_ERROR("Failed to find unit view with id %u", id);
            return;
        }

        view->update();
        UIControls::updateRouteManager();
    } else {
        LOG_ERROR("Failed to set midi thru for unit %u to %s",
                    resp.toggleMidiThru.unit->id(),
                    (resp.toggleMidiThru.newState ? "true" : "false"));
    }
}

inline void handleToggleOmniHwInputResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Omni hw input for unit %u is set to %s", 
                    resp.toggleOmniHwInput.unit->id(),
                    (resp.toggleOmniHwInput.newState ? "true" : "false"));

        const slr::ID id = resp.toggleOmniHwInput.unit->id();
        AudioUnitView * view = ctx.projectView->getUnitById(id);
        if(!view) {
            LOG_ERROR("Failed to find unit view with id %u", id);
            return;
        }

        view->update();
        UIControls::updateRouteManager();
    } else {
        LOG_ERROR("Failed to set midi thru for unit %u to %s",
                    resp.toggleOmniHwInput.unit->id(),
                    (resp.toggleOmniHwInput.newState ? "true" : "false"));
    }
}

inline void handleAppendItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
		LOG_INFO("Item %u appended successfully on unit id: %u", resp.appendItem.item->_uniqueId, resp.appendItem.unit->id());
		// ProjectView * psnap = ControlEngine::getLastSnapshot();
		
		AudioUnitView * uview = ctx.projectView->getUnitById(resp.appendItem.unit->id());
		// TrackView * tv = static_cast<TrackView*>(uview);
		if(!uview) {
			LOG_ERROR("Failed to find AudioUnitView with id %u", resp.appendItem.unit->id());
			return;
		}

		ContainerItemView * item = new ContainerItemView(resp.appendItem.item);
		uview->_fileList._items.push_back(item);
		// ctx.projectView->getTrack(resp.appendItem.track->id())->_fileList._items.push_back(item);
		
		// UIControls::addFileToTrackUI(tv->id(), tv->_fileList._items.back());
		UIControls::updateModuleUI(uview->id());
		// swapSnapshot();
	} else { 
		LOG_ERROR("Need to handle error!");
	}
}

inline void handleContainerSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for unit %u swapped, deleting old one", resp.swapContainer.unit->id());
        delete resp.swapContainer.oldContainer;
    } else {
        LOG_ERROR("Failed to swap unit %u container", resp.swapContainer.unit->id());
    }
}

inline void handleModifyContainerItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::NotOk) {
        LOG_ERROR("Failed to modify container item:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.unit->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));
        return;
    }

    LOG_INFO("Container item modified:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.unit->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));

    AudioUnitView * uview = ctx.projectView->getUnitById(resp.modContainerItem.unit->id());
    if(!uview) {
        LOG_ERROR("Failed to find AudioUnitView for unit %u", resp.modContainerItem.unit->id());
        return;  
    }
    // TrackView * tv = static_cast<TrackView*>(uview);
	// if(!tv) {
	// 	LOG_ERROR("Failed to find TrackView with id %u", resp.appendItem.track->id());
	// 	return;
	// }

    std::vector<ContainerItemView*> items = uview->_fileList._items;
    ContainerItemView * item = nullptr;
    for(std::size_t i=0; i<items.size(); ++i) {
        if(resp.modContainerItem.item->_uniqueId == items.at(i)->_uniqueId) {
            item = items.at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Failed to find ContainerItemView for item %u", resp.modContainerItem.item->_uniqueId);
        return;
    }

    item->_startPosition = resp.modContainerItem.item->_startPosition;
    item->_length = resp.modContainerItem.item->_length;
    item->_muted = resp.modContainerItem.item->_muted;

    UIControls::updateModuleUI(uview->id());
}

inline void handleEvent(const ControlContext &ctx, const Events::SetParameter &e) {
    LOG_INFO("Set parameter event. Target: %u, parameter: %u, value: %f", 
                    e.targetId, e.parameterId, e.value);
    AudioUnit * unit = ctx.project->getUnitById(e.targetId);
    if(!unit) {
        LOG_ERROR("No target with such id %u", e.targetId);
        return;
    }

    if(!unit->hasParameterWithId(e.parameterId)) {
        LOG_ERROR("No such parameter %u for unit %u", e.parameterId, e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::SetParameter;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.setParameter.unit = unit;
    ctl.setParameter.parameterId = e.parameterId;
    ctl.setParameter.value = e.value;
    ControlEngine::emitRtControl(ctl);
}

inline void handleEvent(const ControlContext &ctx, const Events::ToggleMidiThru &e) {
    LOG_INFO("Toggling midi thru for unit %u new state %s",
                e.targetId, (e.newState ? "true" : "false"));

    AudioUnit *u = ctx.project->getUnitById(e.targetId);
    if(!u) {
        LOG_ERROR("Failed to find unit with id %u", e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ToggleMidiThru;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.toggleMidiThru.unit = u;
    ctl.toggleMidiThru.newState = e.newState;
    ControlEngine::emitRtControl(ctl);
}

inline void handleEvent(const ControlContext &ctx, const Events::ToggleOmniHwInput &e) {
    LOG_INFO("Toggling midi thru for unit %u new state %s",
                e.targetId, (e.newState ? "true" : "false"));

    AudioUnit *u = ctx.project->getUnitById(e.targetId);
    if(!u) {
        LOG_ERROR("Failed to find unit with id %u", e.targetId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ToggleOmniHwInput;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.toggleOmniHwInput.unit = u;
    ctl.toggleOmniHwInput.newState = e.newState;
    ControlEngine::emitRtControl(ctl);
}

inline void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());


	AudioUnit * u = ctx.project->getUnitById(e.targetId);
	if(!u) {
		LOG_ERROR("No unit with such id %u", e.targetId);
		return;
	}
	// Track * trg = dynamic_cast<Track*>(u);
	// if(!trg) {
	// 	LOG_ERROR("Failed to convert to Track ptr");
	// 	return;
	// }

    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.targetId;
	ctx.fileWorker->addTask(std::move(task));
}

inline void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Status::Ok) {
		LOG_INFO("File %s opened with id %u, adding to target unit %u", e.file->path().c_str(), e.file->id(), e.targetId);

		ContainerItem * item = new ContainerItem(e.file);
		// item->_file = e.file;
		item->_startPosition = 0;
		item->_length = e.file->frames();
		item->_muted = false;
		// item->_uniqueId = e.file->id();

		AudioUnit * u = ctx.project->getUnitById(e.targetId);
		if(!u) {
			LOG_ERROR("No unit with such id %u", e.targetId);
			return;
		}
		// Track * trg = static_cast<Track*>(u);
		// if(!trg) {
		// 	LOG_ERROR("Failed to convert to Track ptr");
		// 	return;
		// }

		std::size_t size = u->items()->size();
		if(u->items()->size()+1 > u->items()->capacity()) {
			//create new vector
			LOG_INFO("Unit id:%u container run out of space, resizing", e.targetId);
			std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
			container->reserve(size*2);
			*container = *u->items();

			container->push_back(item);

			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.swapContainer.unit = u;
			ctrl.swapContainer.container = container;
   
			ControlEngine::awaitRtResult(ctrl, [item](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
				FlatEvents::FlatResponse fake;
				fake.type = FlatEvents::FlatResponse::Type::AppendItem;
				fake.status = resp.status;
				fake.appendItem.unit = resp.swapContainer.unit;
				fake.appendItem.item = item;
				handleAppendItemResponse(ctx, fake);
			});
		} else {
			//just append
			LOG_INFO("Appending item %u to unit id: %u", item->_uniqueId, e.targetId);
			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::AppendItem;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.appendItem.unit = u;
			ctrl.appendItem.item = item;
			ControlEngine::emitRtControl(ctrl);
		}
	} else {
        //TODO: failed to open
		LOG_ERROR("Failed to open audio file");
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::RemoveFile &e) {    
    LOG_INFO("Removing file UI %u from unit %u", e.fileId, e.unitId);
    // ProjectView * snap = ctx.projectView;
    // TrackView * trview = snap->getTrack(e.trackId);
    AudioUnitView * uview = ctx.projectView->getUnitById(e.unitId);
    // TrackView * trview = static_cast<TrackView*>(uview);
    if(uview == nullptr) {
        LOG_ERROR("Failed to find AudioUnitView for track: %u", e.unitId);
        return;
    }
    //need to remove containerview as well
    //find ContainerItemView
    std::size_t idxViews = 0;
    ContainerItemView * itemView = nullptr;
    std::vector<ContainerItemView*> & itemsViews = uview->_fileList._items;
    for(auto & itm : itemsViews) {
        if(itm->_uniqueId == e.fileId) {   
            itemView = itm;
            break;
        }
        idxViews++;
    } 

    if(itemView == nullptr) {
        LOG_ERROR("failed to find FileView for file %u in track %u", e.fileId, e.unitId);
        return;
    }

    //remove
    // UIControls::removeFileFromTrackUI(trview, e.fileId);
    itemsViews.erase(itemsViews.begin()+idxViews);
    delete itemView;
    
    UIControls::updateModuleUI(e.unitId);
}

inline void handleEvent(const ControlContext &ctx, const Events::FileUIRemoved &e) { 
    LOG_INFO("Removing file id %u from unit id %u", e.fileId, e.unitId);
    //remove
    AudioUnit * unit = ctx.project->getUnitById(e.unitId);
    // Track * tr = dynamic_cast<Track*>(unit);
    if(unit == nullptr) {
        LOG_ERROR("Failed to find unit, id: %u is wrong or unit doesn't exists", e.unitId);
        return;
    }

    ContainerItem * item = nullptr;
    const std::vector<ContainerItem*> * items = unit->items();
    std::size_t idx = 0;
    for(std::size_t i=0; i<items->size(); ++i) {
        if(items->at(i)->_file->id() == e.fileId) {
            item = items->at(i);
            idx = i;
            break;
        }
    }

    if(item == nullptr) {
        LOG_ERROR("Failed to find file with id %u in unit %u", e.fileId, e.unitId);
        return;
    }

    //prepare swapable container
    std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
    *container = *items;
    //TODO: must check that item removed from container
    container->erase(container->begin()+idx);

    FlatEvents::FlatControl swap;
    swap.type = FlatEvents::FlatControl::Type::SwapContainer;
    swap.commandId = ControlEngine::generateCommandId();
    swap.swapContainer.unit = unit;
    swap.swapContainer.container = container;

    ControlEngine::awaitRtResult(swap, [](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status == Status::Ok) {
            LOG_INFO("Item removed from unit %u successfully", resp.swapContainer.unit->id());
        } else { 
            LOG_ERROR("Failed to remove item from unit %u", resp.swapContainer.unit->id());
        }
    });
}

inline void handleEvent(const ControlContext &ctx, const Events::ModContainerItem &e) {
    LOG_INFO("Modify Container Item Event: unit %u, item %u, start %lu, length %lu, muted %s",
                    e.unitId,
                    e.itemId,
                    e.startPosition,
                    e.length,
                    (e.muted ? "true" : "false"));
    
    AudioUnit * unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Modify Container Item failed to find unit %u", e.unitId);
        return;
    }

    // Track * track = static_cast<Track*>(unit);
    // if(!track) {
    //     LOG_ERROR("Unit is not a track");
    //     return;
    // }

    const std::vector<ContainerItem*> *items = unit->items();
    ContainerItem * item = nullptr;
    for(std::size_t i=0; i<items->size(); ++i) {
        if(items->at(i)->_uniqueId == e.itemId) {
            item = items->at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Modify Container Item failed to find item %u in unit %u", e.itemId, e.unitId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ModContainerItem;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.modContainerItem.unit = unit;
    ctl.modContainerItem.item = item;
    ctl.modContainerItem.startPosition = e.startPosition;
    ctl.modContainerItem.length = e.length;
    ctl.modContainerItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}

} //namespace slr
