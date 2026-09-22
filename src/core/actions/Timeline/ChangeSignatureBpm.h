// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"

#include <memory>
#include <atomic>
#include <vector>

namespace slr {

struct ActionBase;
struct AudioUnit;
struct Timeline;
struct ClipItem;
struct AudioFile;
struct MidiFile;

struct ChangeSignatureBpmAction : public ActionExecutable, public Undoable {
    ChangeSignatureBpmAction(const ActionBase *base);
    ~ChangeSignatureBpmAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::ChangeSignatureBpm _action;
    Actions::ChangeSignatureBpm _oldValues;


    struct ChangeSigBpm : public FlatTask {
        void execRT();

        Timeline * tl;
        float bpm;
        BarSize sig;
        std::vector<AudioFile*> audiofiles;
        std::vector<MidiFile*> midifiles;
        std::vector<ClipItem*> clips;
        std::atomic<bool> completed;
    };

    ChangeSigBpm _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> createChangeSignatureBpmAction(const ActionBase*);

}
