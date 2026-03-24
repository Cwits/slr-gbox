// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/ControlContext.h"
#include "core/primitives/AudioUnit.h"

namespace slr {

ID ControlContext::nextAudioUnitId() const {
    return AudioUnit::nextAudioUnitId();
}

}
