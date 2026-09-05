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

struct UndoAction : public ActionExecutable {
    UndoAction(const ActionBase *base);
    ~UndoAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::Undo _action;
};

std::unique_ptr<ActionExecutable> createUndoAction(const ActionBase*);

}