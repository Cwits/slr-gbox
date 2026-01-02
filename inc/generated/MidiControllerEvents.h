/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"
#include <string>
#include <unordered_map>
#include "Status.h"
#include "core/MidiController.h"

namespace slr {

class RtEngine;

namespace Events {

struct ToggleMidiDevice {
    MidiDevice * device;
    MidiSubdevice * subdev;
    DevicePort port;
    bool newState;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct UpdateMidiMaps {
    RtEngine * engine; //-> class RtEngine;
    std::vector<RtMidiBuffer> *localBuffers;
    std::vector<RtMidiQueue> *inputMap;
    std::vector<RtMidiOutput> *outputMap;
};

} //namespace FlatControls

namespace FlatResponses {

struct UpdateMidiMaps {
    std::vector<RtMidiBuffer> *oldLocal;
    std::vector<RtMidiQueue> *oldInput;
    std::vector<RtMidiOutput> *oldOutput;
    Status status;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
