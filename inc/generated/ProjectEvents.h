/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/AudioRoute.h"
#include "defines.h"

namespace slr {


namespace Events {

struct NewTrack {
};
struct DeleteTrack {
    ID targetId;
};
struct AddNewRoute {
    AudioRoute route;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {


} //namespace FlatControls

namespace FlatResponses {


} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
