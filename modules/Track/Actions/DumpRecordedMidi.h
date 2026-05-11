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

struct ActionNameAction : public ActionExecutable {
    ActionNameAction(const ActionBase *base);
    ~ActionNameAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::ActionName _action;

    struct FlatAction : public FlatTask {
        void execRT();

        <variables>
        std::atomic<bool> completed;
    };

    FlatAction _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createActionNameAction(const ActionBase*);

}

