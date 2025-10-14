// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK RemoveFile 
/* 
    Only removes file reference from track, but not closes it, so file stays open for now...
*/

EV
INCLUDE <cstdint> 
INCLUDE "defines.h"
struct RemoveFile {
    ID fileId;
    ID targetId;
};

EV
INCLUDE "defines.h"
struct FileUIRemoved {
    ID fileId;
    ID targetId;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/primitives/File.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::RemoveFile &e) {    
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
END_HANDLE

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "modules/Track/TrackView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/primitives/File.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::FileUIRemoved &e) { 
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
END_HANDLE

END_BLOCK