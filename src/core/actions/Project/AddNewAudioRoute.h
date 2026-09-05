// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;

struct AddNewRouteAction : public ActionExecutable, public Undoable {
    AddNewRouteAction(const ActionBase *base);
    ~AddNewRouteAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::AddNewAudioRoute _action;

    RtTasks::SwapRenderPlan _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createAddNewRouteAction(const ActionBase*);

}
