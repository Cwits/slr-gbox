// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/actions/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct AudioUnitView;

struct CreateNewUnitAction : public ActionExecutable, public Undoable {
    CreateNewUnitAction(const ActionBase *base);
    ~CreateNewUnitAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::CreateNewUnit _action;
    ID _createdUnitId;

    AudioUnitView * _view;
};

std::unique_ptr<ActionExecutable> createCreateNewUnitAction(const ActionBase*);

}
