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

struct ChangeTimelineStateAction : public ActionExecutable {
    ChangeTimelineStateAction(const ActionBase *base);
    ~ChangeTimelineStateAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ChangeTimelineState _action;

    struct ChangeTimelineState : public FlatTask {
        void execRT();

        Timeline *tl;
        TimelineState state;
        std::atomic<bool> completed;
    };

    ChangeTimelineState _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createChangeTimelineStateAction(const ActionBase*);

}
