// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"
#include "common/defines.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct Timeline;

struct LoopPositionAction : public ActionExecutable, public Undoable {
    LoopPositionAction(const ActionBase *base);
    ~LoopPositionAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::LoopPosition _action;
    Actions::LoopPosition _oldValues;

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
