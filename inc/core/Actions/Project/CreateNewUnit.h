// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;

struct CreateNewUnitAction : public ActionExecutable {
    CreateNewUnitAction(const ActionBase *base);
    ~CreateNewUnitAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::CreateNewUnit _action;
};

std::unique_ptr<ActionExecutable> createCreateNewUnitAction(const ActionBase*);

}
