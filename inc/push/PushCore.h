// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/MidiEvent.h"
#include "push/PushDisplayInterface.h"

namespace slr {
    class MidiDevice;
    class MidiSubdevice;
    class MidiPort;
}

namespace PushLib {

struct PushCore {
    PushCore();
    ~PushCore();

    bool connect(slr::MidiDevice *dev, slr::MidiSubdevice *subdev, slr::MidiPort * port);
    void reconnect();
    void disconnect();

    DisplayInterface _display;

    private:
    slr::MidiDevice * _dev;
    slr::MidiSubdevice * _subdev;
    slr::MidiPort * _port;

    void handleRealTimeEvent(const slr::MidiEventType type);
    void handleSysexEvent(const unsigned char *data, const int &len);
    void handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data);

    friend void pushThreadLoop(PushLib::PushCore * push);
};

}