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
    ID unitId;
};

EV
INCLUDE "defines.h"
struct FileUIRemoved {
    ID fileId;
    ID unitId;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
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
INCLUDE <algorithm>
void handleEvent(const ControlContext &ctx, const Events::FileUIRemoved &e) { 
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
END_HANDLE

END_BLOCK