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

struct ToggleOmniHwAction : public ActionExecutable {
    ToggleOmniHwAction(const ActionBase *base);
    ~ToggleOmniHwAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::ToggleOmniHwInput _action;

    struct ToggleOmniHw : public FlatTask {
        void execRT();

        AudioUnit *target;
        bool newState;
        std::atomic<bool> completed;
    };

    ToggleOmniHw _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createToggleOmniHwAction(const ActionBase*);

}

