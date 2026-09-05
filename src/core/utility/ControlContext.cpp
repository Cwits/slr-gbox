// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/utility/ControlContext.h"
#include "core/primitives/AudioUnit.h"
#include "core/RtEngine.h"

namespace slr {

ID ControlContext::nextAudioUnitId() const {
    return AudioUnit::nextAudioUnitId();
}

void ControlContext::EmitRtTask(RtTask * task) {
    _nonConstEngine->addTask(task);
}

}
