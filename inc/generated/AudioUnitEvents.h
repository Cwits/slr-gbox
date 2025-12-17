/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"

namespace slr {

class AudioUnit;

namespace Events {

struct SetParameter {
    ID targetId;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    ID targetId;
    bool newState;
};
struct ToggleOmniHwInput {
    ID targetId;
    bool newState;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct ToggleOmniHwInput {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};

} //namespace FlatControls

namespace FlatResponses {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct ToggleOmniHwInput {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
