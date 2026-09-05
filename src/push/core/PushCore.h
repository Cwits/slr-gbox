// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/core/PushDisplay.h"
#include "push/core/PushPainter.h"
#include "push/core/PushContext.h"
#include "push/core/PushMidi.h"
#include "push/core/PushSysex.h"
#include "push/core/PushLeds.h"
#include "push/core/PushPads.h"
#include "push/core/PushLib.h"

#include <atomic>

namespace slr {
    class MidiPort;
}

namespace PushLib {

class Widget;

struct PushCore {
    PushCore();
    ~PushCore();

    bool connect(slr::MidiPort * port);
    void reconnect();
    void disconnect();
    bool connected() const { return _connected; }

    PushContext * context() { return &_context; }

    void setRootWidget(Widget * w) { _rootWidget = w; _manualRedraw = true; }
    void tick(int dt);

    void redraw() { _manualRedraw = true; }

    private:
    std::atomic<bool> _connected;

    PushMidi _midi;
    PushSysex _sysex;
    Painter _painter;
    PushDisplay _display;
    PushLeds _leds;
    PushPads _pads;
    PushContext _context;

    bool _manualRedraw;
    Widget * _rootWidget;

    friend class PushMidi;
};

}