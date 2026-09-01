// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/primitives/StepEvent.h"
#include "core/Actions.h"

#include <vector>
#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct Sequence;
struct AudioUnit;

struct ModifySequenceTargetAction : public ActionExecutable {
    ModifySequenceTargetAction(const ActionBase *base);
    ~ModifySequenceTargetAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifySequenceTarget _action;

    struct ModifyEvent : public FlatTask {
        void execRT();

        Sequence * editable;
        const AudioUnit * target;
        bool addTarget; //false to remove;

        std::atomic<bool> completed;
    };

    ModifyEvent _flatModEvent;
    RtTask _task;
    // std::unique_ptr<Sequence> _beforeClone;
};

std::unique_ptr<ActionExecutable> createModifySequenceTargetAction(const ActionBase*);

}