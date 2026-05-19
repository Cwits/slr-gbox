// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"
#include "units/Sampler/SamplerActions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioFile;
struct Sampler;

struct LoadAssetAction : public ActionExecutable {
    LoadAssetAction(const ActionBase *base);
    ~LoadAssetAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::LoadAsset _action;

    struct FileOpened { 
        FileOpened() { file = nullptr; result = false; completed.store(false); }

        const File *file;
        bool result;
        std::atomic<bool> completed;
    }; 
    FileOpened _opened;

    struct LoadAssetFlat : public FlatTask {
        void execRT();

        const AudioFile * file;
        Sampler * sampler;
        std::atomic<bool> completed;
    };

    LoadAssetFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createLoadAssetAction(const ActionBase*);

}