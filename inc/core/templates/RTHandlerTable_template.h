// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//GENERATE_INCLUDES

#include "core/FlatEvents.h"
#include "Status.h"
namespace slr {

namespace RTHandlers {

using RTHandler = Status(*)(const FlatEvents::FlatControl&, FlatEvents::FlatResponse&);
extern "C" {
constexpr RTHandler RTTable[] = {
    //GENERATE_RT_HANDLES
};
}

} //namespace RTHandlers

} //namespace slr