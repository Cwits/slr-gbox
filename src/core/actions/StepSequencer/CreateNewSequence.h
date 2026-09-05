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
struct Sequence;
struct StepSequencerEngine;

struct CreateNewSequenceAction : public ActionExecutable, public Undoable {
    CreateNewSequenceAction(const ActionBase *base);
    ~CreateNewSequenceAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::CreateNewSequence _action;

    struct AddFlat : public FlatTask {
        void execRT();

        StepSequencerEngine * engine;
        Sequence * newSeq;
        std::atomic<bool> completed;
    };

    struct RemoveFlat : public FlatTask {
        void execRT();
        
        StepSequencerEngine * engine;
        Sequence * tofind;
        std::atomic<bool> completed;
    };

    AddFlat _flat;
    RemoveFlat _rflat;
    RtTask _task;

    Sequence * _createdPtr;
};

std::unique_ptr<ActionExecutable> createCreateNewSequenceAction(const ActionBase*);

}
