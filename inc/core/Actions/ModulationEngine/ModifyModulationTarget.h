// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"
#include "common/ModEngineCommon.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct ModulationPattern;
struct ModulationEngine;
struct ParameterBase;

struct ModifyModulationTargetAction : public ActionExecutable, public Undoable {
    ModifyModulationTargetAction(const ActionBase *base);
    ~ModifyModulationTargetAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifyModulationTarget _action;

    struct ModifyModulation : public FlatTask {
        void execRT();

        // ModulationPattern * modPtrn;
        // ModTargetArray arr;

        ParameterBase * base;
        bool addOrRemove; //true on add
        const float * ptr; 
        int ammount;

        std::atomic<bool> completed;
    };

    ModTargetArray _toRestore;
    // Actions::ModifyModulation _toRestore;
    ModifyModulation _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createModifyModulationTargetAction(const ActionBase*);

}