// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/primitives/ClipContainer.h"
#include "core/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
// struct ClipContainer;

struct LoadAsClipAction : public ActionExecutable {
    LoadAsClipAction(const ActionBase *base);
    ~LoadAsClipAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    // void undo(ControlContext &ctx) override;
    // void redo(ControlContext &ctx) override;

    private:
    const Actions::LoadAsClip _action;

    const File * _file;
    const ClipItem * _clip;

    struct FileOpened {
        FileOpened() { file = nullptr; result = false; completed.store(false); }

        const File * file;
        bool result;
        std::atomic<bool> completed;
    };

    FileOpened _opened;

    RtTasks::SwapContainerFlat _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createLoadAsClipAction(const ActionBase*);

}
