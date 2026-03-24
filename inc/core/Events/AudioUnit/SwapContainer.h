// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SwapContainer

FLAT_REQ
INCLUDE "core/primitives/FileContainer.h"
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit; //-> class AudioUnit;
    const ClipContainer * container; //-> class ClipItem;
};

FLAT_RESP
INCLUDE "core/primitives/FileContainer.h"
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit;
    const ClipContainer * oldContainer; //-> class ClipItem;
    const ClipContainer * newContainer;
};
 
RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::swapContainer
END_HANDLE

RESP_HANDLE
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleContainerSwappedNew(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Common::Status::Ok) {
        LOG_INFO("Container for unit %u swapped", resp.swapContainer.unit->id());
    } else {
        LOG_ERROR("Failed to swap unit %u container", resp.swapContainer.unit->id());
    }
}
END_HANDLE

END_BLOCK