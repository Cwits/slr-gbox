// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK RemoveClip 
/* 
    Only removes file reference from track, but not closes it, so file stays open for now...
*/

EV
INCLUDE <cstdint> 
INCLUDE "defines.h"
struct RemoveClip {
    ID clipId;
    ID unitId;
};

EV
INCLUDE "defines.h"
struct ClipUIRemoved {
    ID clipId;
    ID unitId;
};

EV_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::RemoveClip &e) {    
    LOG_INFO("Removing clip UI %u from unit %u", e.clipId, e.unitId);

    AudioUnitView * uview = ctx.projectView->getUnitById(e.unitId);
    if(uview == nullptr) {
        LOG_ERROR("Failed to find AudioUnitView for track: %u", e.unitId);
        return;
    }

    uview->_clipContainer.deleteClipViewItem(e.clipId);
    
    // UIControls::fileRemovedFromId(uview->id());
}
END_HANDLE

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
INCLUDE <algorithm>
void handleEvent(const ControlContext &ctx, const Events::ClipUIRemoved &e) { 
    LOG_INFO("Removing file id %u from unit id %u", e.clipId, e.unitId);
    //remove
    AudioUnit * unit = ctx.project->getUnitById(e.unitId);
    if(unit == nullptr) {
        LOG_ERROR("Failed to find unit, id: %u is wrong or unit doesn't exists", e.unitId);
        return;
    }

    ClipItem * item = nullptr;
    const ClipContainer * clips = unit->clips();
    auto it = std::find_if(
        clips->begin(), 
        clips->end(), 
        [clipid = e.clipId](const ClipItem *i) {
        return i->id() == clipid;
    });

    if(it != clips->end()) item = *it;

    if(item == nullptr) {
        LOG_ERROR("Failed to find file with id %u in unit %u", e.clipId, e.unitId);
        return;
    }

    ClipContainerBuffer &storage = ctx.project->getClipContainerBufferById(e.unitId);
    ClipContainer *modifiable = storage.modifiableContainer();

    modifiable->clear();
    if(modifiable->capacity() < clips->capacity()) {
        modifiable->resize(clips->capacity());
    }

    *modifiable = *clips;

    modifiable->erase(
        std::remove_if(
            modifiable->begin(),
            modifiable->end(),
            [item](const ClipItem *i) {
                return i == item;
            }
    ), modifiable->end());

    FlatEvents::FlatControl swap;
    swap.type = FlatEvents::FlatControl::Type::SwapContainer;
    swap.swapContainer.unit = unit;
    swap.swapContainer.container = modifiable;

    ControlEngine::awaitRtResult(swap, [itemId = item->id()](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status == Common::Status::NotOk) {
            LOG_ERROR("Failed to remove item from unit %u", resp.swapContainer.unit->id());
            LOG_ERROR("Unhandled state");
            return;
        }
        
        /* 
            not deleting Clip and closing file totally.
            May add some reference counter to file and some kind of clean up process to close and free such memory.
            otherwise they stay in memory so user can reuse them quickly(need some kind of access procedure for it)
        */
        // const ID & unitId = resp.swapContainer.unit->id();

        //remove from memory owner
        // ClipContainerMap &map = ctx.project->clipContainerMap();
        // ClipStorage & clipStorage = map.at(unitId);
        // ClipContainerBuffer &clipStorage = ctx.project->getClipContainerBufferById(unitId);

        // std::vector<std::unique_ptr<ClipItem>> &owner = clipStorage._clipsOwner;
        // owner.erase(std::remove_if(owner.begin(), owner.end(), [itemId](const std::unique_ptr<ClipItem> & itm) {
        //     return itemId == itm->_uniqueId;
        // }), owner.end());

        LOG_INFO("Clip %u removed from unit %u successfully", itemId, resp.swapContainer.unit->id());
    });
}
END_HANDLE

END_BLOCK