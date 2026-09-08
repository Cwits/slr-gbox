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
struct ModulationPattern;
struct ModulationEngine;
struct RenderPlan;

struct CreateNewModulationAction : public ActionExecutable {
    CreateNewModulationAction(const ActionBase *base);
    ~CreateNewModulationAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::CreateNewModulation _action;

    ModulationPattern * _createdPtrn;

    struct NewModulation : public FlatTask {
        void execRT();

        // <variables>
        ModulationEngine * engine;
        ModulationPattern * createdPtrn;
        std::atomic<bool> completed;
    };

    RtTasks::SwapRenderPlan _flatSwap;

    struct RemoveMod : public FlatTask {
        void execRT();
        
        ModulationEngine * engine;
        ModulationPattern * createdPtrn;
        std::atomic<bool> completed;
    };

    NewModulation _flat;
    RemoveMod _rflat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createCreateNewModulationAction(const ActionBase*);

}