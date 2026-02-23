// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/MidiEvent.h"
#include "push/PushDisplayInterface.h"
#include "push/PushPainter.h"
#include "push/PushLib.h"

#include <queue>
#include <mutex>
#include <thread>
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

    void setMainWidget(Widget * w) { _mainWidget = w; _manualRedraw = true; }
    void tick(int dt);

    void handleRealTimeEvent(const slr::MidiEventType type);
    void handleSysexEvent(const unsigned char *data, const int &len);
    void handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data);

    private:
    bool _connected;
    slr::MidiPort * _port;

    Painter _painter;
    DisplayInterface _display;

    bool dispatchEvents(); 

    std::mutex _pushMutex;
    std::queue<ButtonEvent> _buttonQueue;
    std::queue<ButtonEvent> _localButtonQueue;

    bool _manualRedraw;
    Widget * _mainWidget;
};

}