// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK OpenFile 

EV
INCLUDE <string>
INCLUDE "defines.h"
struct OpenFile {
	ID unitId;
    std::string path;
};

EV
INCLUDE "Status.h"
INCLUDE "defines.h"
struct FileOpened {
    Status status;
    const File * file; //-> class File;
	ID unitId;
};

FLAT_REQ
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ClipItem * item; //-> class ClipItem;
};


EV_HANDLE
INCLUDE "core/FileTasks.h"
INCLUDE "core/FileWorker.h"
INCLUDE <memory>
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());

	AudioUnit * u = ctx.project->getUnitById(e.unitId);
	if(!u) {
		LOG_ERROR("No unit with such id %u", e.unitId);
		return;
	}

    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.unitId;
	task->finished = [](bool success, const ID targetId, const File * file, const std::string & path) {
		if(!success) {
			LOG_ERROR("File Worker failed to load file %s", path.c_str());
			return;
		}

		Events::FileOpened e = {
            .status = slr::Status::Ok,
            .file = file,
            .unitId = targetId
        };

        EmitEvent(e);
        LOG_INFO("File %s loaded to trackid: %d", path.c_str(), targetId);
	};
	ctx.fileWorker->addTask(std::move(task));
}
END_HANDLE

EV_HANDLE
INCLUDE "Status.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
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
	auto [it, inserted] = map.try_emplace(e.unitId);
	ClipStorage &clipStorage = it->second;

	bool isSwapping = false;
	std::vector<ClipItem*> *workable = nullptr;
	const std::vector<ClipItem*> *clips = unit->clips();
	if(!clips) {
		// workable = clipStorage._rawVector1.get();
		workable = clipStorage.getOtherVector(nullptr);
		workable->reserve(4);
		isSwapping = true;
	} else {
		if(clips->size()+1 > clips->capacity()) {
			workable = clipStorage.getOtherVector(clips);
			workable->clear();
			workable->reserve(clips->capacity()*2);
			*workable = *clips;
			isSwapping = true;
		} else {
			workable = clipStorage.getCurrentVector(clips);
		}
	}

	ClipItem * appendable = nullptr;
	std::unique_ptr<ClipItem> itemUniq = std::make_unique<ClipItem>(e.file);
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
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "logger.h"  
void handleAppendItemNewResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
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
END_HANDLE

END_BLOCK