// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/MidiEvent.h"
#include "defines.h"
#include <atomic>

namespace slr {

struct StepEvent {
    bool _enabled;
    MidiEvent _event;

    void clear() {
        _enabled = false;
        _event.clear();
    }

    StepEvent& operator=(const StepEvent &rhs) {
        _enabled = rhs._enabled;
        _event = rhs._event;
        return *this;
    }
};


}