// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;

struct LoadProjectAction : public ActionExecutable {
    LoadProjectAction(const ActionBase *base);
    ~LoadProjectAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::LoadProject _action;

    nlohmann::json _dataToLoad;

    //step1 - creating modules
    int _foundUnits;
    int _unitsNotFound;
};

std::unique_ptr<ActionExecutable> createLoadProjectAction(const ActionBase*);

}
