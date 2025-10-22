/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"
#include <string>
#include "core/primitives/AudioRoute.h"

namespace slr {


namespace Events {

struct AddNewRoute {
    AudioRoute route;
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
