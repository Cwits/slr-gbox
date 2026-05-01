// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"
#include "defines.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct Timeline;

struct LoopPositionAction : public ActionExecutable {
    LoopPositionAction(const ActionBase *base);
    ~LoopPositionAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::LoopPosition _action;

    struct SetLoopPosition : public FlatTask {
        void execRT();

        Timeline *tl;
        frame_t start;
        frame_t end;
        std::atomic<bool> completed;
    };
    SetLoopPosition _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createLoopPositionAction(const ActionBase*);

}
