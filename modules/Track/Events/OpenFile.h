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
	Track * track; //-> class Track;
	ContainerItem * item; //->class ContainerItem;
};

EV_HANDLE
INCLUDE "core/FileTasks.h"
INCLUDE "core/FileWorker.h"
INCLUDE <memory>
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());


	AudioUnit * u = ctx.project->getUnitById(e.targetId);
	if(!u) {
		LOG_ERROR("No unit with such id %u", e.targetId);
		return;
	}
	Track * trg = dynamic_cast<Track*>(u);
	if(!trg) {
		LOG_ERROR("Failed to convert to Track ptr");
		return;
	}

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
INCLUDE "modules/Track/Track.h"
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Status::Ok) {
		LOG_INFO("File %s opened with id %u, adding to target track %u", e.file->path().c_str(), e.file->id(), e.targetId);

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
		Track * trg = static_cast<Track*>(u);
		if(!trg) {
			LOG_ERROR("Failed to convert to Track ptr");
			return;
		}

		std::size_t size = trg->items()->size();
		if(trg->items()->size()+1 > trg->items()->capacity()) {
			//create new vector
			LOG_INFO("Track id:%u container run out of space, resizing", e.targetId);
			std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
			container->reserve(size*2);
			*container = *trg->items();

			container->push_back(item);

			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.swapContainer.track = trg;
			ctrl.swapContainer.container = container;
   
			ControlEngine::awaitRtResult(ctrl, [item](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
				FlatEvents::FlatResponse fake;
				fake.type = FlatEvents::FlatResponse::Type::AppendItem;
				fake.status = resp.status;
				fake.appendItem.track = resp.swapContainer.track;
				fake.appendItem.item = item;
				handleAppendItemResponse(ctx, fake);
			});
		} else {
			//just append
			LOG_INFO("Appending item %u to track id: %u", item->_uniqueId, e.targetId);
			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::AppendItem;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.appendItem.track = trg;
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
INCLUDE "modules/Track/Track.h"
&Track::appendItem
END_HANDLE

RESP_HANDLE
INCLUDE "Status.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "ui/uiControls.h"
INCLUDE "modules/Track/Track.h"
INCLUDE "logger.h"  
void handleAppendItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
		LOG_INFO("Item %u appended successfully on track id: %u", resp.appendItem.item->_uniqueId, resp.appendItem.track->id());
		// ProjectView * psnap = ControlEngine::getLastSnapshot();
		
		AudioUnitView * uview = ctx.projectView->getUnitById(resp.appendItem.track->id());
		TrackView * tv = static_cast<TrackView*>(uview);
		if(!tv) {
			LOG_ERROR("Failed to find TrackView with id %u", resp.appendItem.track->id());
			return;
		}

		ContainerItemView * item = new ContainerItemView(resp.appendItem.item);
		tv->_fileList._items.push_back(item);
		// ctx.projectView->getTrack(resp.appendItem.track->id())->_fileList._items.push_back(item);
		
		// UIControls::addFileToTrackUI(tv->id(), tv->_fileList._items.back());
		UIControls::updateModuleUI(tv->id());
		// swapSnapshot();
	} else { 
		LOG_ERROR("Need to handle error!");
	}
}
END_HANDLE

END_BLOCK