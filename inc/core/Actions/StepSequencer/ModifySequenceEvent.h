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

struct ModifySequenceEventAction : public ActionExecutable {
    ModifySequenceEventAction(const ActionBase *base);
    ~ModifySequenceEventAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifySequenceEvent _action;

    struct ModifyEvent : public FlatTask {
        void execRT();

        Sequence * editable;
        unsigned int layer;
        unsigned int eventNum;

        bool enabled;
        uint8_t note;
        uint8_t velocity;
        
        std::atomic<bool> completed;
    };

    ModifyEvent _flatModEvent;
    RtTask _task;
    std::unique_ptr<Sequence> _beforeClone;
};

std::unique_ptr<ActionExecutable> createModifySequenceEventAction(const ActionBase*);

}