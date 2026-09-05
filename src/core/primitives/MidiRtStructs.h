// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiBuffer.h"
#include "core/primitives/MidiPort.h"

namespace slr {

//for use in RT only
struct RtMidiQueue {
    ID id;
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> *queue;
};

struct RtMidiOutput {
    RtMidiOutput(MidiPort *port, ID id) : id(id), port(port) {}
    const ID id;
    void sendEvent(const MidiEvent &ev) {
        port->sendMidi(ev);
        // if(!port->outputOpened()) return;

        // bool wasEmpty = port->outQueue()->empty();
        // port->outQueue()->push(ev);
        // if(wasEmpty) {
        //     uint64_t one = 1;
        //     write(port->_efd, &one, sizeof(one));
        // }
    }

    private:
    MidiPort *port;
};


}