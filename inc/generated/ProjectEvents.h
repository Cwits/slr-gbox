/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/MidiRoute.h"
#include "core/primitives/AudioRoute.h"
#include <string>
#include "defines.h"

namespace slr {


namespace Events {

struct AddNewRoute {
    AudioRoute route;
};
struct AddNewMidiRoute {
    MidiRoute route;
};
struct DeleteModule {
    ID targetId;
};
struct CreateModule {
    std::string name;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {


} //namespace FlatControls

namespace FlatResponses {


} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
