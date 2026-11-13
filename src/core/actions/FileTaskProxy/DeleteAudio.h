// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"

#include "core/primitives/AudioBuffer.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioFile;

struct DeleteAudioAction : public ActionExecutable {
    DeleteAudioAction(const ActionBase *base);
    ~DeleteAudioAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::DeleteAudio _action;

    std::atomic<bool> _completed;
    AudioFile * _swapTarget = nullptr;
    std::unique_ptr<AudioBuffer> _newBuffer;
    
    struct SwapAudioFileData : public FlatTask {
        void execRT();

        std::unique_ptr<AudioBuffer> buf;
        AudioFile * file;

        std::atomic<bool> completed;
    };

    SwapAudioFileData _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createDeleteAudioAction(const ActionBase*);

}
