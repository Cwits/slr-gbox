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

struct ChangeSignatureBpmAction : public ActionExecutable, public Undoable {
    ChangeSignatureBpmAction(const ActionBase *base);
    ~ChangeSignatureBpmAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    void undo(ControlContext &ctx) override;
    void redo(ControlContext &ctx) override;

    private:
    const Actions::ChangeSignatureBpm _action;
    Actions::ChangeSignatureBpm _oldValues;


    struct ChangeSigBpm : public FlatTask {
        void execRT();

        Timeline * tl;
        float bpm;
        BarSize sig;
        std::atomic<bool> completed;
    };

    ChangeSigBpm _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createChangeSignatureBpmAction(const ActionBase*);

}
