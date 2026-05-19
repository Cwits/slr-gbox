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
struct Timeline;

struct ToggleLoopAction : public ActionExecutable {
    ToggleLoopAction(const ActionBase *base);
    ~ToggleLoopAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ToggleLoop _action;

    struct ToggleFlat : public FlatTask {
        void execRT();

        Timeline *tl;
        bool state;
        std::atomic<bool> completed;
    };
    ToggleFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createToggleLoopAction(const ActionBase*);

}
