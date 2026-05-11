// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"

#include "core/MidiController.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;
struct RtEngine; 

struct ToggleMidiDeviceAction : public ActionExecutable {
    ToggleMidiDeviceAction(const ActionBase *base);
    ~ToggleMidiDeviceAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition() override;

    private:
    const Actions::ToggleMidiDevice _action;

    struct UpdateMidiMaps : public FlatTask {
        void execRT();

        RtEngine * engine;
        std::vector<RtMidiBuffer> *localBuffers;
        std::vector<RtMidiQueue> *inputMap;
        std::vector<RtMidiOutput> *outputMap;

        const std::vector<RtMidiBuffer> *oldLocal;
        const std::vector<RtMidiQueue> *oldInput;
        const std::vector<RtMidiOutput> *oldOutput;
        
        std::atomic<bool> completed;
    };

    UpdateMidiMaps _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createToggleMidiDeviceAction(const ActionBase*);

}