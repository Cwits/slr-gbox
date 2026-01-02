/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/ControlEngine.h"
#include <memory>
#include <algorithm>
#include "logger.h"  
#include "core/FlatEvents.h"
#include "core/primitives/AudioUnit.h"
#include "snapshots/ProjectView.h"
#include "core/primitives/AudioUnit.h"  
#include "Status.h"
#include "core/FileWorker.h"
#include "snapshots/AudioUnitView.h"
#include "core/Project.h"
#include <string>
#include "core/primitives/FileContainer.h"
#include "ui/uiControls.h"
#include "inc/ui/uiControls.h"
#include "core/primitives/File.h"
#include "core/FileTasks.h"
#include "logger.h"
#include "snapshots/FileContainerView.h"
#include "modules/Track/TrackView.h"
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

inline void handleAppendItemNewResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
	if(resp.status == Status::Ok) {
		LOG_INFO("Item %u appended successfully on unit id: %u", resp.appendItem.item->_uniqueId, resp.appendItem.unit->id());
		// ProjectView * psnap = ControlEngine::getLastSnapshot();
		
		AudioUnitView * uview = ctx.projectView->getUnitById(resp.appendItem.unit->id());
		// TrackView * tv = static_cast<TrackView*>(uview);
		if(!uview) {
			LOG_ERROR("Failed to find AudioUnitView with id %u", resp.appendItem.unit->id());
			return;
		}

		ClipItemView * item = new ClipItemView(resp.appendItem.item);
		uview->_clipContainer._items.push_back(item);

		UIControls::updateModuleUI(uview->id());
	} else {    
		LOG_ERROR("Need to handle error!");
	}
}

inline void handleModifyClipItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::NotOk) {
        LOG_ERROR("Failed to modify clip item:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modClipItem.unit->id(),
                    resp.modClipItem.item->_uniqueId,
                    resp.modClipItem.startPosition,
                    resp.modClipItem.length,
                    (resp.modClipItem.muted ? "true" : "false"));
        return;
    }

    LOG_INFO("Clip item modified:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modClipItem.unit->id(),
                    resp.modClipItem.item->_uniqueId,
                    resp.modClipItem.startPosition,
                    resp.modClipItem.length,
                    (resp.modClipItem.muted ? "true" : "false"));

    AudioUnitView * uview = ctx.projectView->getUnitById(resp.modClipItem.unit->id());
    if(!uview) {
        LOG_ERROR("Failed to find AudioUnitView for unit %u", resp.modClipItem.unit->id());
        return;  
    }

    std::vector<ClipItemView*> items = uview->_clipContainer._items;
    ClipItemView * item = nullptr;
    for(std::size_t i=0; i<items.size(); ++i) {
        if(resp.modClipItem.item->_uniqueId == items.at(i)->_uniqueId) {
            item = items.at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Failed to find ClipItemView for item %u", resp.modClipItem.item->_uniqueId);
        return;
    }

    item->_startPosition = resp.modClipItem.item->startPosition();
    item->_length = resp.modClipItem.item->length();
    item->_muted = resp.modClipItem.item->isMuted();

    UIControls::updateModuleUI(uview->id());
}

inline void handleContainerSwappedNew(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for unit %u swapped, deleting old one", resp.swapContainer.unit->id());
        // delete resp.swapContainerNew.oldContainer;
    } else {
        LOG_ERROR("Failed to swap unit %u container", resp.swapContainer.unit->id());
    }
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

	AudioUnit * u = ctx.project->getUnitById(e.unitId);
	if(!u) {
		LOG_ERROR("No unit with such id %u", e.unitId);
		return;
	}

    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.unitId;
	task->fileStartPosition = e.fileStartPosition;
	task->finished = [](bool success, const ID targetId, const File * file, const std::string & path, const frame_t fileStartPosition) {
		if(!success) {
			LOG_ERROR("File Worker failed to load file %s", path.c_str());
			return;
		}

		Events::FileOpened e = {
            .status = slr::Status::Ok,
            .file = file,
            .unitId = targetId,
			.fileStartPosition = fileStartPosition
        };

        EmitEvent(e);
        LOG_INFO("File %s loaded to trackid: %d", path.c_str(), targetId);
	};
	ctx.fileWorker->addTask(std::move(task));
}

inline void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Status::NotOk) {
        //TODO: failed to open
		LOG_ERROR("Failed to open audio file");
        return;
    }

	LOG_INFO("File %s opened with id %u, adding to target unit %u", e.file->path().c_str(), e.file->id(), e.unitId);
    AudioUnit *unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Failed to find unit %u", e.unitId);
        return;
    }

	ClipContainerMap &map = ctx.project->clipContainerMap();
	// auto [it, inserted] = map.try_emplace(e.unitId);
	ClipStorage &clipStorage = map.at(e.unitId);
	// ClipStorage &clipStorage = it->second;

	bool isSwapping = false;
	std::vector<ClipItem*> *workable = nullptr;
	const std::vector<ClipItem*> *clips = unit->clips();
	if(clips->size()+1 > clips->capacity()) {
		workable = clipStorage.getOtherVector(clips);
		workable->clear();
		workable->reserve(clips->capacity()*2);
		*workable = *clips;
		isSwapping = true;
	} else {
		workable = clipStorage.getCurrentVector(clips);
	}
	// if(!clips) {
	// 	// workable = clipStorage._rawVector1.get();
	// 	workable = clipStorage.getOtherVector(nullptr);
	// 	workable->reserve(4);
	// 	isSwapping = true;
	// } else {
	// 	if(clips->size()+1 > clips->capacity()) {
	// 		workable = clipStorage.getOtherVector(clips);
	// 		workable->clear();
	// 		workable->reserve(clips->capacity()*2);
	// 		*workable = *clips;
	// 		isSwapping = true;
	// 	} else {
	// 		workable = clipStorage.getCurrentVector(clips);
	// 	}
	// }

	ClipItem * appendable = nullptr;
	std::unique_ptr<ClipItem> itemUniq = std::make_unique<ClipItem>(e.file, e.fileStartPosition);
	clipStorage._clipsOwner.push_back(std::move(itemUniq));
	appendable = clipStorage._clipsOwner.back().get();
	
    if(isSwapping) {
        //swap one
		LOG_INFO("Unit id:%u container run out of space, resizing", e.unitId);
		
		workable->push_back(appendable);

		FlatEvents::FlatControl ctrl;
		ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
		ctrl.commandId = ControlEngine::generateCommandId();
		ctrl.swapContainer.unit = unit;
		ctrl.swapContainer.container = workable;
   
		ControlEngine::awaitRtResult(ctrl, [appendable](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
			FlatEvents::FlatResponse fake;
			fake.type = FlatEvents::FlatResponse::Type::AppendItem;
			fake.status = resp.status;
			fake.appendItem.unit = resp.swapContainer.unit;
			fake.appendItem.item = appendable;
			handleAppendItemNewResponse(ctx, fake);
		});
    } else {
        //append?
		LOG_INFO("Appending item %u to unit id: %u", appendable->_uniqueId, e.unitId);
		FlatEvents::FlatControl ctrl;
		ctrl.type = FlatEvents::FlatControl::Type::AppendItem;
		ctrl.commandId = ControlEngine::generateCommandId();
		ctrl.appendItem.unit = unit;
		ctrl.appendItem.item = appendable;
		ControlEngine::emitRtControl(ctrl);
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::RemoveFile &e) {    
    LOG_INFO("Removing file UI %u from unit %u", e.fileId, e.unitId);
    
    AudioUnitView * uview = ctx.projectView->getUnitById(e.unitId);
    if(uview == nullptr) {
        LOG_ERROR("Failed to find AudioUnitView for track: %u", e.unitId);
        return;
    }
    //need to remove containerview as well
    //find ContainerItemView
    std::size_t idxViews = 0;
    ClipItemView * itemView = nullptr;
    std::vector<ClipItemView*> & itemsViews = uview->_clipContainer._items;
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
    if(unit == nullptr) {
        LOG_ERROR("Failed to find unit, id: %u is wrong or unit doesn't exists", e.unitId);
        return;
    }

    ClipItem * item = nullptr;
    const std::vector<ClipItem*> * clips = unit->clips();
    std::size_t idx = 0;
    for(std::size_t i=0; i<clips->size(); ++i) {
        if(clips->at(i)->_file->id() == e.fileId) {
            item = clips->at(i);
            idx = i;
            break;
        }
    }

    if(item == nullptr) {
        LOG_ERROR("Failed to find file with id %u in unit %u", e.fileId, e.unitId);
        return;
    }

    ClipContainerMap &map = ctx.project->clipContainerMap();
	ClipStorage &clipStorage = map.at(e.unitId); //??? eeeegh...

    std::vector<ClipItem*> * swappable = clipStorage.getOtherVector(clips);
    swappable->clear();
    *swappable = *clips;

    swappable->erase(swappable->begin()+idx);

    FlatEvents::FlatControl swap;
    swap.type = FlatEvents::FlatControl::Type::SwapContainer;
    swap.commandId = ControlEngine::generateCommandId();
    swap.swapContainer.unit = unit;
    swap.swapContainer.container = swappable;

    ControlEngine::awaitRtResult(swap, [itemId = item->_uniqueId](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status == Status::NotOk) 
            LOG_ERROR("Failed to remove item from unit %u", resp.swapContainer.unit->id());
        
        const ID & unitId = resp.swapContainer.unit->id();

        //remove from memory owner
        ClipContainerMap &map = ctx.project->clipContainerMap();
        ClipStorage & clipStorage = map.at(unitId);

        std::vector<std::unique_ptr<ClipItem>> &owner = clipStorage._clipsOwner;
        owner.erase(std::remove_if(owner.begin(), owner.end(), [itemId](const std::unique_ptr<ClipItem> & itm) {
            return itemId == itm->_uniqueId;
        }), owner.end());

        LOG_INFO("Item %u removed from unit %u successfully", itemId, resp.swapContainer.unit->id());
    });
}

inline void handleEvent(const ControlContext &ctx, const Events::ModClipItem &e) {
    LOG_INFO("Modify Clip Item Event: unit %u, item %u, start %lu, length %lu, muted %s",
                    e.unitId,
                    e.itemId,
                    e.startPosition,
                    e.length,
                    (e.muted ? "true" : "false"));
    
    AudioUnit * unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Modify Clip Item failed to find unit %u", e.unitId);
        return;
    }

    const std::vector<ClipItem*> *items = unit->clips();
    ClipItem * item = nullptr;
    for(std::size_t i=0; i<items->size(); ++i) {
        if(items->at(i)->_uniqueId == e.itemId) {
            item = items->at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Modify Clip Item failed to find item %u in unit %u", e.itemId, e.unitId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ModClipItem;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.modClipItem.unit = unit;
    ctl.modClipItem.item = item;
    ctl.modClipItem.startPosition = e.startPosition;
    ctl.modClipItem.length = e.length;
    ctl.modClipItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}

} //namespace slr
