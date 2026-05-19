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

struct SetParameterAction : public ActionExecutable {
    SetParameterAction(const ActionBase *base);
    ~SetParameterAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::SetParameter _action;

    RtTasks::SetParameterFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createSetParameterAction(const ActionBase*);

}
