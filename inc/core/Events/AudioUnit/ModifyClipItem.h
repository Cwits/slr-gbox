// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE "defines.h"
struct ModClipItem {
    ID unitId;
    ID itemId;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

FLAT_REQ
INCLUDE "defines.h"
struct ModClipItem { 
    AudioUnit * unit; //-> class AudioUnit;
    ClipItem * item; //-> class ClipItem;
    frame_t startPosition;
    frame_t length;
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
    ctl.modClipItem.unit = unit;
    ctl.modClipItem.item = item;
    ctl.modClipItem.startPosition = e.startPosition;
    ctl.modClipItem.length = e.length;
    ctl.modClipItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::modifyClipItem
END_HANDLE

RESP_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "ui/uiControls.h"
INCLUDE "Status.h"
INCLUDE "logger.h"
void handleModifyClipItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
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

    // UIControls::updateModuleUI(uview->id());
}
END_HANDLE

END_BLOCK