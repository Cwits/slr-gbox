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

struct RedoAction : public ActionExecutable {
    RedoAction(const ActionBase *base);
    ~RedoAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::Redo _action;
};

std::unique_ptr<ActionExecutable> createRedoAction(const ActionBase*);

}