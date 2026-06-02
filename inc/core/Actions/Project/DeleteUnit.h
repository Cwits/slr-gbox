// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionExecutable.h"
#include "core/primitives/RtTask.h"
#include "core/Actions.h"
#include "core/Serializer.h"
#include "core/UnitManager.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"

#include <memory>
#include <atomic>
#include <string>

namespace UI {
    struct UnitUIBase;
}

namespace PushUI {

}

namespace slr {

struct ActionBase;
struct AudioUnit;
struct Project;
struct BufferManager;

struct DeleteUnitAction : public ActionExecutable, public Undoable {
    DeleteUnitAction(const ActionBase *base);
    ~DeleteUnitAction();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    void undo(ControlContext &ctx) override;
    void redo(ControlContext &ctx) override;

    private:
    const Actions::DeleteUnit _action;
        
    RtTasks::SwapRenderPlan _flat;
    RtTask _task;

    std::unique_ptr<AudioUnit> _unit;
    std::shared_ptr<AudioUnitView> _unitView;
    std::vector<AudioRoute> _audioRoutes;
    std::vector<MidiRoute> _midiRoutes;

    BufferManager * _bmanptr;
};	

std::unique_ptr<ActionExecutable> createDeleteUnitAction(const ActionBase*);

}
