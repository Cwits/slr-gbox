// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE "defines.h"
struct ModClipItem {
    ID clipId;
    frame_t startPosition;
    frame_t length;
    frame_t fileStartOffset;
    bool muted;
};

FLAT_REQ
INCLUDE "defines.h"
struct ModClipItem { 
    ClipItem * item; //-> class ClipItem;
    frame_t startPosition;
    frame_t length;
    frame_t fileStartOffset;
    bool muted;
};
  
EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/primitives/FileContainer.h" 
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::ModClipItem &e) {
    LOG_INFO("Modify Clip Item Event: clip %u, start %lu, fileStartOffset %lu, length %lu, muted %s",
                    e.clipId,  
                    e.startPosition,
                    e.fileStartOffset,
                    e.length,
                    (e.muted ? "true" : "false"));

    ClipItem * item = ctx.project->findClipItemById(e.clipId);
    if(!item) {
        LOG_ERROR("Failed to find clip item with id %d", e.clipId);
        return;
    }

    //do some checks here?? like what?

    
    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ModClipItem;
    ctl.modClipItem.item = item;
    ctl.modClipItem.startPosition = e.startPosition;
    ctl.modClipItem.length = e.length;
    ctl.modClipItem.fileStartOffset = e.fileStartOffset;
    ctl.modClipItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/Project.h"
&Project::modifyClipItem
END_HANDLE

RESP_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "ui/uiControls.h"
INCLUDE "common/Status.h"
INCLUDE "logger.h"
void handleModifyClipItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Common::Status::NotOk) {
        LOG_ERROR("Failed to modify clip item: item %u, start %lu, length %lu, file offset %lu, muted %s",
                    resp.modClipItem.item->id(),
                    resp.modClipItem.startPosition,
                    resp.modClipItem.length,
                    resp.modClipItem.fileStartOffset,
                    (resp.modClipItem.muted ? "true" : "false"));
        return;
    }

    // AudioUnitView * uview = ctx.projectView->getUnitById(resp.modClipItem.unit->id());
    // if(!uview) {
    //     LOG_ERROR("Failed to find AudioUnitView for unit %u", resp.modClipItem.unit->id());
    //     return;  
    // }

    ClipItemView * item = ctx.projectView->findClipViewById(resp.modClipItem.item->id());
    if(!item) {
        LOG_ERROR("Failed to find ClipItemView with id %lu", resp.modClipItem.item->id());
        return;
    }

    item->update();
    // std::vector<ClipItemView*> items = uview->_clipContainer._items;
    // ClipItemView * item = nullptr;
    // for(std::size_t i=0; i<items.size(); ++i) {
    //     if(resp.modClipItem.item->_uniqueId == items.at(i)->_uniqueId) {
    //         item = items.at(i);
    //         break;
    //     }
    // }

    // if(!item) {
    //     LOG_ERROR("Failed to find ClipItemView for item %u", resp.modClipItem.item->_uniqueId);
    //     return;
    // }

    // item->_startPosition = resp.modClipItem.item->startPosition();
    // item->_length = resp.modClipItem.item->length();
    // item->_muted = resp.modClipItem.item->isMuted();

    // // UIControls::updateModuleUI(uview->id());
}
END_HANDLE

END_BLOCK