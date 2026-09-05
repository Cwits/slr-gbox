// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "units/Track/TrackActions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;

struct RecordArmAction : public ActionExecutable {
    RecordArmAction(const ActionBase *base);
    ~RecordArmAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::RecordArm _action;

    struct RecordArmFlat : public FlatTask {
        void execRT();

        Track * track;
        float recordState;
        RecordSource recordSource;
        std::atomic<bool> completed;
    };

    RecordArmFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createRecordArmAction(const ActionBase*);

}
