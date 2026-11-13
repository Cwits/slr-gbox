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

struct CopyAudioAction : public ActionExecutable {
    CopyAudioAction(const ActionBase *base);
    ~CopyAudioAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::CopyAudio _action;

    // struct FlatAction : public FlatTask {
    //     void execRT();

    //     <variables>
    //     std::atomic<bool> completed;
    // };

    // FlatAction _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createCopyAudioAction(const ActionBase*);

}
