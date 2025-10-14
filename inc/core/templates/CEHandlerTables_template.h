// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//GENERATE_INCLUDES

#include "core/Events.h"
#include "core/FlatEvents.h"
#include "core/primitives/ControlContext.h"

namespace slr {

namespace Handlers {

using EventHandlerFn = void(*)(const ControlContext &ctx, const Events::Event&);
using ResponseHandlerFn = void(*)(const ControlContext &ctx, const FlatEvents::FlatResponse&);

template<typename T>
void dispatchHelper(const ControlContext &ctx, const Events::Event& ev) {
    const auto& e = std::get<T>(ev);
    handleEvent(ctx, e); // calls the specific handler
}

extern "C" {
constexpr EventHandlerFn ControlTable[] = {
    //&dispatchHelper<Events::RecordArm>,
    //GENERATE_CONTROL_HANDLES
};
}

extern "C" {
constexpr ResponseHandlerFn ResponseTable[] = {
    //&handleAppendItemResponse,
    //GENERATE_RESPONSE_HANDLES
};
}

} //Handlers

} //namespace slr