// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK OpenFile 

EV
INCLUDE <string>
INCLUDE "defines.h"
struct OpenFile {
	ID targetId;
    std::string path;
};

EV
INCLUDE "Status.h"
INCLUDE "defines.h"
struct FileOpened {
    Status status;
    File * file; //-> class File;
	ID targetId;
};

FLAT_REQ
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ContainerItem * item; //-> class ContainerItem;
};

EV_HANDLE
INCLUDE "core/FileTasks.h"
INCLUDE "core/FileWorker.h"
INCLUDE <memory>
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());
    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.targetId;
	ctx.fileWorker->addTask(std::move(task));
}
END_HANDLE

EV_HANDLE
INCLUDE "Status.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "core/FlatEvents.h"
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
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
END_HANDLE

RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::appendItem
END_HANDLE

RESP_HANDLE
INCLUDE "Status.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "ui/uiControls.h"
INCLUDE "logger.h"  
void handleAppendItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
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
END_HANDLE

END_BLOCK