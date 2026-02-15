#pragma once

// #include "PushMidi.h"
#include ""

namespace PushLib {

struct PushCore {
    PushCore();
    ~PushCore();

    void connect();
    void reconnect();
    void disconnect();

    // MidiInterface _midi;
    // SysexInterface _sysex;
    // LedInterface _led;
    // PadInterface _pads;

    // DisplayInterface _display;


    private:

};

}