// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/MidiController/VirtualMidiKbdAction.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/primitives/MidiEvent.h"
#include "core/MidiController.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

VMKTriggerAction::VMKTriggerAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::VMKTrigger*>(base)) )
{

}

VMKTriggerAction::~VMKTriggerAction() {

}

void VMKTriggerAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(_action.note < 1 || _action.note > 127) {
        LOG_ERROR("Wrong note value %d expected >= 1 and <= 127", _action.note); 
        abortAction();
        return; 
    }

    if(_action.velocity < 0 || _action.velocity > 127) {
        LOG_ERROR("Wrong note velocity %d expected >= 0 and <= 127", _action.velocity); 
        abortAction();
        return;
    }

    if(_action.channel < 0 || _action.channel > 15) {
        LOG_ERROR("Wrong note channel %d expected >= 0 and <= 15", _action.channel); 
        abortAction();
        return; 
    }

    slr::MidiEvent ev;
    ev.type = _action.isPressed ? slr::MidiEventType::NoteOn : slr::MidiEventType::NoteOff;
    ev.channel = _action.channel;
    ev.note = _action.note;
    ev.velocity = _action.velocity;
    ev.offset = 0;

    ctx.midiController->addVirtualKbdEvent(ev);

    setState(ActionState::Finished);
}

void VMKTriggerAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

}


std::unique_ptr<ActionExecutable> createVMKTriggerAction(const ActionBase *base) {
    return std::make_unique<VMKTriggerAction>(base);
}

}