// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushDisplay.h"
#include "push/PushPainter.h"
#include "push/PushContext.h"
#include "push/PushMidi.h"
#include "push/PushSysex.h"
#include "push/PushLeds.h"
#include "push/PushPads.h"
#include "push/PushLib.h"

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

    void setMainWidget(Widget * w) { _mainWidget = w; _manualRedraw = true; }
    void tick(int dt);

    void redraw() { _manualRedraw = true; }

    private:
    bool _connected;

    PushMidi _midi;
    PushSysex _sysex;
    Painter _painter;
    PushDisplay _display;
    PushLeds _leds;
    PushPads _pads;
    PushContext _context;

    bool _manualRedraw;
    Widget * _mainWidget;

    friend class PushMidi;
};

}