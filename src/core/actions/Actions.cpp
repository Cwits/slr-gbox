// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/Actions.h"

#include "core/actions/ActionBase.h"
#include "core/ControlEngine.h"

namespace slr {

void EmitAction(std::unique_ptr<ActionBase> action) {
    ControlEngine::EmitAction(std::move(action));
}

}