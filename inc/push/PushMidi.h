// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
#include "push/Color.h"

#include "core/primitives/MidiEvent.h"
#include "core/primitives/SPSCQueue.h"

#include <vector>
#include <mutex>
#include <queue>

namespace slr {
    class MidiPort;
}

namespace PushLib {

class PushCore;
class PushSysex;

struct PushMidi {
    PushMidi(PushCore &core, PushSysex &sysex);
    ~PushMidi();

    bool connect(slr::MidiPort * port);
    bool reconnect();
    bool disconnect();

    void sendSingle(slr::MidiEvent &ev);
    void sendBatch(std::vector<slr::MidiEvent> &v);

    void handleRealTimeEvent(const slr::MidiEventType type);
    void handleSysexEvent(const unsigned char *data, const int &len);
    void handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data);

    bool dispatchEvents();

    private:
    PushCore &_core;
    PushSysex &_sysex;

    slr::MidiPort * _port;

    std::mutex _pushMutex;
    // std::queue<ButtonEvent> _buttonQueue;
    slr::SPSCQueue<ButtonEvent, 32> _buttonQueue;
    std::queue<ButtonEvent> _localButtonQueue;
    // std::queue<SysexMsg> _sysexQueue;

    slr::SPSCQueue<SysexMsg, 32> _sysexQueue;
};

}