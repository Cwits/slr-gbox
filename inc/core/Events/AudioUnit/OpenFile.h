// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK OpenFile 

EV
INCLUDE <string>
INCLUDE "defines.h"
struct OpenFile {
	ID unitId;
    std::string path;
	frame_t fileStartPosition;  
};

EV
INCLUDE "common/Status.h"
INCLUDE "defines.h"
struct FileOpened {
    Common::Status status;
    const File * file; //-> class File;
	ID unitId;
	frame_t fileStartPosition;
};

EV_HANDLE
INCLUDE "core/FileTasks.h"
INCLUDE "core/FileWorker.h"
INCLUDE <memory>
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());

	if(ctx.prohibitAllocation(0)) {
		LOG_WARN("Low on memory, prohibit creating new things");
		return;
	}

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
            .status = Common::Status::Ok,
            .file = file,
            .unitId = targetId,
			.fileStartPosition = fileStartPosition
        };

        EmitEvent(e);
        LOG_INFO("File %s loaded to trackid: %d", path.c_str(), targetId);
	};
	ctx.fileWorker->addTask(std::move(task));
}
END_HANDLE

EV_HANDLE
INCLUDE "common/Status.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "logger.h"  
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Common::Status::NotOk) {
		LOG_ERROR("Failed to open audio file");
        return;
    }

	LOG_INFO("File %s opened with id %u, adding to target unit %u", e.file->path().c_str(), e.file->id(), e.unitId);
    AudioUnit *unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Failed to find unit %u", e.unitId);
        return;
    }

	ClipContainerBuffer &unitStorage = ctx.project->getClipContainerBufferById(e.unitId);

	ClipContainer *modContainer = unitStorage.modifiableContainer();
	const ClipContainer *inUseContainer = unitStorage.inUseContainer();

	if(inUseContainer->size()+1 > inUseContainer->capacity()) {
		if(ctx.prohibitAllocation(inUseContainer->capacity()*2*sizeof(ClipItem*))) {
			LOG_WARN("Low on memory, prohibit creating new things");
			return;
		}
		modContainer->clear();
		modContainer->reserve(inUseContainer->capacity()*2);
		*modContainer = *(inUseContainer);
	} else {
		modContainer->clear();
		*modContainer = *(inUseContainer);
	}

	if(ctx.prohibitAllocation(sizeof(ClipItem))) {
		LOG_WARN("Low on memory, prohibit creating new things");
		return;
	}
  
	ClipItem *clip = ctx.project->clipStorage().newClip(e.file, e.fileStartPosition);
	// ClipItem *clip = ctx.project->createClip(e.file, e.fileStartPosition); 

	if(!clip) {
		LOG_WARN("Failed to create new clip from file %d", e.file->id());
		return;
	}

	modContainer->push_back(clip);

	FlatEvents::FlatControl ctrl;
	ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
	ctrl.swapContainer.unit = unit;
	ctrl.swapContainer.container = modContainer;
   
	
	ControlEngine::awaitRtResult(ctrl, [clip](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
		if(resp.status == Common::Status::NotOk) {
			LOG_ERROR("Need to handle error!");
		}
		
		LOG_INFO("Item %u appended successfully on unit id: %u", resp.swapContainer.newContainer->back()->id(), resp.swapContainer.unit->id());
		ClipContainerBuffer &unitStorage = ctx.project->getClipContainerBufferById(resp.swapContainer.unit->id());
		unitStorage.containerSwapped();

		AudioUnitView * uview = ctx.projectView->getUnitById(resp.swapContainer.unit->id());
		if(!uview) {
			LOG_ERROR("Failed to find AudioUnitView with id %u", resp.swapContainer.unit->id());
			return;
		}

		if(ctx.prohibitAllocation(sizeof(ClipItemView))) {
			LOG_WARN("Low on memory");
			return;
		}

		ClipItemView *cview = ctx.projectView->createClipView(clip);
		if(!cview) {
			LOG_WARN("Failed to create ClipView");
			//need some cleanup
			return;
		}

		uview->_clipContainer.addClipItem(cview);
		
		// UIControls::fileAddedToId(uview->id());
	});
}
END_HANDLE


END_BLOCK