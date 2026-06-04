// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;

struct SetColorAction : public ActionExecutable, public Undoable {
    SetColorAction(const ActionBase *base);
    ~SetColorAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::SetColor _action;
    Color _oldColor;
};

std::unique_ptr<ActionExecutable> createSetColorAction(const ActionBase*);

}