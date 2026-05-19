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
struct Project;

struct DeleteUnitAction : public ActionExecutable, public Undoable {
    DeleteUnitAction(const ActionBase *base);
    ~DeleteUnitAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    void undo(ControlContext &ctx) override;
    void redo(ControlContext &ctx) override;

    private:
    const Actions::DeleteUnit _action;
        
    RtTasks::SwapRenderPlan _flat;
    RtTask _task;
};	

std::unique_ptr<ActionExecutable> createDeleteUnitAction(const ActionBase*);

}
