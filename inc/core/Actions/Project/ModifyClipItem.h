// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct ClipItem;

struct ModifyClipItemAction : public ActionExecutable, public Undoable {
    ModifyClipItemAction(const ActionBase *base);
    ~ModifyClipItemAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifyClipItem _action;
    Actions::ModifyClipItem _oldValues;

    struct ModifyClip : public FlatTask {
        void execRT();

        ClipItem * item;
        frame_t startPosition;
        frame_t length;
        frame_t fileStartOffset;
        bool muted;
        std::atomic<bool> completed;
    };

    ModifyClip _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createModifyClipItemAction(const ActionBase*);

}
