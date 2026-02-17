/* This file is generated automatically, do not edit manually */
#pragma once
#include <functional>
#include <string>
#include "defines.h"
#include "core/MidiController.h"
#include "Status.h"
#include <unordered_map>

namespace slr {

class RtEngine;

namespace Events {

struct ToggleMidiDevice {
    MidiDevice * device;
    MidiSubdevice * subdev;
    DevicePort port;
    bool newState;
    std::function<void(int)> completed;
};
struct VirtualMidiKbdAction {
    int note;
    int velocity;
    int channel;
    bool isPressed;
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
