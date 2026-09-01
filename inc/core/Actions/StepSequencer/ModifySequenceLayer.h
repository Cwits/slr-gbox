// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/primitives/StepEvent.h"
#include "core/Actions.h"

#include <vector>
#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct Sequence;

struct ModifySequenceLayerAction : public ActionExecutable {
    ModifySequenceLayerAction(const ActionBase *base);
    ~ModifySequenceLayerAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ModifySequenceLayer _action;

    struct ModifyLayer : public FlatTask {
        void execRT();

        Sequence * editable;
        unsigned int layer;
        uint8_t note;
        uint8_t velocity;
        bool mute;
        bool active;
        const AudioUnit * target;

        std::atomic<bool> completed;
    };

    ModifyLayer _flatModLayer;
    RtTask _task;
    // std::unique_ptr<Sequence> _beforeClone;
};

std::unique_ptr<ActionExecutable> createModifySequenceLayerAction(const ActionBase*);

}