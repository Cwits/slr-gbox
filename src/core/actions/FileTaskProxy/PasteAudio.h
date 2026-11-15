// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/Actions.h"
#include "core/actions/ActionExecutable.h"
#include "core/primitives/RtTask.h"

#include "core/primitives/AudioBuffer.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioFile;

struct PasteAudioAction : public ActionExecutable {
    PasteAudioAction(const ActionBase *base);
    ~PasteAudioAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::PasteAudio _action;

    std::unique_ptr<AudioBuffer> _readyBuffer;
    AudioFile *_swapTarget = nullptr;
    std::atomic<bool> _finished;

    RtTasks::SwapAudioFileData _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createPasteAudioAction(const ActionBase*);

}
