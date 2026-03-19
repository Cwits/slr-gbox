// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Events.h"
#include "core/ControlEngine.h"

namespace slr {

void EmitEvent(const Events::Event & e) {
    ControlEngine::EmitEvent(e);
    ControlEngine::notify();
}

/* 
    Blocks the execution until timeout reached or got any kind of response
    return true on success, false on fail.
*/
// bool EmitEventBlocking(const Events::Event &e, int msTimeout) {
//     return ControlEngine::EmitEventBlocking(e, msTimeout);
// }

ProjectView * getLastSnapshot() {
    return ControlEngine::projectSnapshot();
}

}