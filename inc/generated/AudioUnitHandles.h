/* This file is generated automatically, do not edit manually */
#pragma once
#include "snapshots/FileContainerView.h"
#include "core/primitives/AudioUnit.h"
#include "core/ControlEngine.h"
#include "core/FileWorker.h"
#include "logger.h"
#include <memory>
#include "core/primitives/AudioUnit.h"  
#include "snapshots/AudioUnitView.h"
#include "core/FlatEvents.h"
#include "logger.h"  
#include "snapshots/ProjectView.h"
#include "modules/Track/TrackView.h"
#include <string>
#include "core/primitives/File.h"
#include "core/primitives/FileContainer.h"
#include "core/FileTasks.h"
#include "ui/uiControls.h"
#include "core/Project.h"
#include "Status.h"
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

inline void handleAppendItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
		LOG_INFO("Item %u appended successfully on unit id: %u", resp.appendItem.item->_uniqueId, resp.appendItem.unit->id());
		
		AudioUnitView * uview = ctx.projectView->getUnitById(resp.appendItem.unit->id());
		if(!uview) {
			LOG_ERROR("Failed to find AudioUnitView with id %u", resp.appendItem.unit->id());
			return;
		}

		ContainerItemView * item = new ContainerItemView(resp.appendItem.item);
		uview->_fileList._items.push_back(item);
		
		UIControls::updateModuleUI(uview->id());
		
	} else { 
		LOG_ERROR("Need to handle error!");
	}
}

inline void handleContainerSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for track %u swapped, deleting old one", resp.swapContainer.unit->id());
        delete resp.swapContainer.oldContainer;
    } else {
        LOG_ERROR("Failed to swap Track container");
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

inline void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());
    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.targetId;
	ctx.fileWorker->addTask(std::move(task));
}

inline void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Status::Ok) {
		LOG_INFO("File %s opened with id %u, adding to target track %u", e.file->path().c_str(), e.file->id(), e.targetId);

		ContainerItem * item = new ContainerItem(e.file);
		item->_startPosition = 0;
		item->_length = e.file->frames();
		item->_muted = false;

		AudioUnit * unit = ctx.project->getUnitById(e.targetId);
		if(!unit) {
			LOG_ERROR("No unit with such id %u", e.targetId);
			return;
		}

		std::size_t size = unit->items()->size();
		if(unit->items()->size()+1 > unit->items()->capacity()) {
			//create new vector
			LOG_INFO("Unit id:%u container run out of space, resizing", e.targetId);
			std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
			container->reserve(size*2);
			*container = *unit->items();

			container->push_back(item);

			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.swapContainer.unit = unit;
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
			ctrl.appendItem.unit = unit;
			ctrl.appendItem.item = item;
			ControlEngine::emitRtControl(ctrl);
		}
	} else {
        //TODO: failed to open
		LOG_ERROR("Failed to open audio file");
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::RemoveFile &e) {    
    LOG_INFO("Removing file UI %u from track %u", e.fileId, e.targetId);

    AudioUnitView * uview = ctx.projectView->getUnitById(e.targetId);
    if(uview == nullptr) {
        LOG_ERROR("Failed to find AudioUnitView for track: %u", e.targetId);
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
        LOG_ERROR("failed to find FileView for file %u in track %u", e.fileId, e.targetId);
        return;
    }

    //remove
    itemsViews.erase(itemsViews.begin()+idxViews);
    delete itemView;
    
    UIControls::updateModuleUI(e.targetId);
}

inline void handleEvent(const ControlContext &ctx, const Events::FileUIRemoved &e) { 
    LOG_INFO("Removing file id %u from track id %u", e.fileId, e.targetId);
    //remove
    AudioUnit * unit = ctx.project->getUnitById(e.targetId);
    if(unit == nullptr) {
        LOG_ERROR("Failed to find track, id: %u is wrong or track doesn't exists", e.targetId);
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
        LOG_ERROR("Failed to find file with id %u in track %u", e.fileId, e.targetId);
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
            LOG_INFO("Item removed from track %u successfully", resp.swapContainer.unit->id());
        } else { 
            LOG_ERROR("Failed to remove item from track %u", resp.swapContainer.unit->id());
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
