// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"

#include <memory>

namespace slr {

struct ActionBase;

struct ToggleMetronomeAction : public ActionExecutable {
    ToggleMetronomeAction(const ActionBase *base);
    ~ToggleMetronomeAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::ToggleMetronome _action;

    RtTasks::SetParameterFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createToggleMetronomeAction(const ActionBase*);

}