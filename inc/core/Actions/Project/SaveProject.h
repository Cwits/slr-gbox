// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;

struct SaveProjectAction : public ActionExecutable {
    SaveProjectAction(const ActionBase *base);
    ~SaveProjectAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::SaveProject _action;
};

std::unique_ptr<ActionExecutable> createSaveProjectAction(const ActionBase*);

}
