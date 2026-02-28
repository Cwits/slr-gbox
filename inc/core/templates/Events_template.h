// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//GENERATE_INCLUDES

#include <variant>

namespace slr {

namespace Events {

using Event = std::variant<
//GENERATE_STRUCTS
>;

} //namespace Events

void EmitEvent(const Events::Event & e);
// bool EmitEventBlocking(const Events::Event &e, int msTimeout); //return false on event failed or timeout(ms)
class ProjectView;
ProjectView * getLastSnapshot();

} //namespace slr