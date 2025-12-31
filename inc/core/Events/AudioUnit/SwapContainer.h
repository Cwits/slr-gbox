// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SwapContainer

FLAT_REQ
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit; //-> class AudioUnit;
    std::vector<ClipItem*> * container; //-> class ClipItem;
};

FLAT_RESP
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit;
    std::vector<ClipItem*> * oldContainer; //-> class ClipItem;
    std::vector<ClipItem*> * newContainer;
};
 
RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::swapContainer
END_HANDLE

RESP_HANDLE
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleContainerSwappedNew(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for unit %u swapped, deleting old one", resp.swapContainer.unit->id());
        // delete resp.swapContainerNew.oldContainer;
    } else {
        LOG_ERROR("Failed to swap unit %u container", resp.swapContainer.unit->id());
    }
}
END_HANDLE

END_BLOCK