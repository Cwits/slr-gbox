// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "modules/Track/TrackActions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;

struct DumpRecAudioAction : public ActionExecutable {
    DumpRecAudioAction(const ActionBase *base);
    ~DumpRecAudioAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::DumpRecordedAudio _action;
};

std::unique_ptr<ActionExecutable> createDumpRecAudioAction(const ActionBase*);

}

