// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/MidiEvent.h"
#include "core/primitives/FixedVector.h"
#include "common/core_config.h"
#include "common/defines.h"

namespace slr {

using MidiBuffer = FixedVector<MidiEvent, MIDI_BUFFER_DEFAULT_SIZE>;
using MidiBufferRecord = FixedVector<MidiEvent, MIDI_BUFFER_RECORD_SIZE>;


struct RtMidiBuffer {
    ID id;
    MidiBuffer * buffer;
};

}