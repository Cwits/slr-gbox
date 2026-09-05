// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct Timeline;

struct UpdatePlayheadAction : public ActionExecutable {
    UpdatePlayheadAction(const ActionBase *base);
    ~UpdatePlayheadAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::UpdatePlayhead _action;

    struct GetPlayhead : public FlatTask {
        void execRT();

        Timeline *tl;
        frame_t position;
        std::atomic<bool> completed;
    };
    GetPlayhead _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createUpdatePlayheadAction(const ActionBase*);

}
