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

struct RemoveClipAction : public ActionExecutable {
    RemoveClipAction(const ActionBase *base);
    ~RemoveClipAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::RemoveClip _action;

    RtTasks::SwapContainerFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createRemoveClipAction(const ActionBase*);

}