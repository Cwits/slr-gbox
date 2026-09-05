// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;

struct SetNameAction : public ActionExecutable, public Undoable {
    SetNameAction(const ActionBase *base);
    ~SetNameAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::SetName _action;
    std::string _oldName;
};

std::unique_ptr<ActionExecutable> createSetNameAction(const ActionBase*);

}
