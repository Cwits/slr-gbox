// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"
#include "common/ModEngineCommon.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct ModulationPattern;
struct ModulationEngine;

struct ModifyModulationAction : public ActionExecutable, public Undoable {
    ModifyModulationAction(const ActionBase *base);
    ~ModifyModulationAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifyModulation _action;

    struct ModifyModulation : public FlatTask {
        void execRT();

        ModulationPattern * modPtrn;
        ModulationShape shape;
        float ampitude;
        float phase;
        float rate;
        bool rateMode;
        float min;
        float max;

        std::atomic<bool> completed;
    };

    Actions::ModifyModulation _toRestore;
    ModifyModulation _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createModifyModulationAction(const ActionBase*);

}