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
struct MidiFile;
struct MidiTrack;

struct AddNewMidiNoteAction : public ActionExecutable {
    AddNewMidiNoteAction(const ActionBase *base);
    ~AddNewMidiNoteAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::AddNewMidiNote _action;

    MidiFile *_editable = nullptr;
    std::unique_ptr<MidiTrack> _swapable;
    std::atomic<bool> _finished;

    struct SwapMidiData : public FlatTask {
        void execRT();

        MidiFile *file;
        std::unique_ptr<MidiTrack> swappable;
        int trackId;
        std::atomic<bool> completed;
    };

    SwapMidiData _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createAddNewMidiNoteAction(const ActionBase*);

}
