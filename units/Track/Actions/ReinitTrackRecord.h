// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"

#include "units/Track/TrackActions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct Track;

struct ReinitTrackRecordAction : public ActionExecutable {
    ReinitTrackRecordAction(const ActionBase *base);
    ~ReinitTrackRecordAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ReinitTrackRecord _action;

    struct ReinitFlat : public FlatTask {
        void execRT();
        
        Track * track;
        std::atomic<bool> completed;
    };

    ReinitFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createReinitTrackRecordAction(const ActionBase*);

}
