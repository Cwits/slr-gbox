// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Events.h"
#include "core/ControlEngine.h"

namespace slr {

void EmitEvent(const Events::Event & e) {
    ControlEngine::EmitEvent(e);
    ControlEngine::notify();
}

ProjectView * getLastSnapshot() {
    return ControlEngine::projectSnapshot();
}

}