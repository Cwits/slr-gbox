// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
#include "push/PushPadLayoutBase.h"

#include "core/primitives/MidiEvent.h"

#include <atomic>

namespace PushLib {

class PushMidi;
class PushSysex;
class PushLeds;

struct PushPads {
    PushPads(PushMidi &midi, PushSysex &sysex, PushLeds &leds);
    ~PushPads();

    const slr::MidiEvent handle(const slr::MidiEventType &type, const int &channel, const unsigned char *data);

    // void setGlobalAftertouchRange(int val);
    // void setGlobalAftertouchMode(AftertouchMode mode);
    // AftertouchMode aftertouchMode();

    // void setGlobalPadVelocityCurve();
    // unsigned char velocityCurve(unsigned char index);

    // void setSensitivity(unsigned char pad, PadSensitivity sens);

    void update();
    const bool needUpdate() const { return _needUpdate; }
    void updated() { _needUpdate = false; }
    void updateRequest() { _needUpdate = true; }

    void setLayout(PushPadLayout * layout) { _layout.store(layout, std::memory_order_relaxed); }
    PushPadLayout * currentLayout() { return _layout; }

    private:
    PushMidi &_midi;
    PushSysex &_sysex; 
    PushLeds &_leds;

    std::atomic<bool> _needUpdate;
    std::atomic<PushPadLayout*> _layout;

};

}