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
struct Sequence;
struct RenderPlan;
struct Timeline;

struct ModifySequenceAction : public ActionExecutable {
    ModifySequenceAction(const ActionBase *base);
    ~ModifySequenceAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifySequence _action;

    struct Modify : public FlatTask {
        void execRT(/* const AudioContext &ctx */);

        const Timeline *tl;
        Sequence * editable;
        int stepCount;
        StepDuration duration;

        std::atomic<bool> completed;
    };

    Modify _flatModify;
    RtTask _task;
    std::unique_ptr<Sequence> _beforeClone;
};

std::unique_ptr<ActionExecutable> createModifySequenceAction(const ActionBase*);

}