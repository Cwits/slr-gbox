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

struct ToggleMidiThruAction : public ActionExecutable {
    ToggleMidiThruAction(const ActionBase *base);
    ~ToggleMidiThruAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ToggleMidiThru _action;

    struct ToggleThru : public FlatTask {
        void execRT();

        AudioUnit *target;
        bool newState;
        std::atomic<bool> completed;
    };

    ToggleThru _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createToggleMidiThruAction(const ActionBase*);

}

