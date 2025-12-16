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

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};

} //namespace FlatControls

namespace FlatResponses {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
