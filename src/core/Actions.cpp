// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions.h"

#include "core/primitives/ActionBase.h"
#include "core/ControlEngine.h"

namespace slr {

void EmitAction(std::unique_ptr<ActionBase> action) {
    ControlEngine::EmitAction(std::move(action));
}

}