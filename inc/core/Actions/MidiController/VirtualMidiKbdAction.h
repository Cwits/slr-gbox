// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/Actions.h"

namespace slr {

struct ActionBase;

struct VMKTriggerAction : public ActionExecutable {
    VMKTriggerAction(const ActionBase *base);
    ~VMKTriggerAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::VMKTrigger _action;

};

std::unique_ptr<ActionExecutable> createVMKTriggerAction(const ActionBase*);

}