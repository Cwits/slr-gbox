// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK 

EV
struct VirtualMidiKbdAction {
    int note;
    int velocity;
    int channel;
    bool isPressed;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/primitives/MidiEvent.h"
INCLUDE "core/MidiController.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::VirtualMidiKbdAction &e) {
    if(e.note < 1 || e.note > 127) { LOG_ERROR("Wrong note value %d expected >= 1 and <= 127", e.note); return; }
    if(e.velocity < 0 || e.velocity > 127) { LOG_ERROR("Wrong note velocity %d expected >= 0 and <= 127", e.velocity); return; }
    if(e.channel < 0 || e.channel > 15) { LOG_ERROR("Wrong note channel %d expected >= 0 and <= 15", e.channel); return; }

    slr::MidiEvent ev;
    ev.type = e.isPressed ? slr::MidiEventType::NoteOn : slr::MidiEventType::NoteOff;
    ev.channel = e.channel;
    ev.note = e.note;
    ev.velocity = e.velocity;
    ev.offset = 0;

    ctx.midiController->addVirtualKbdEvent(ev);
}
END_HANDLE



END_BLOCK