// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/MidiEvent.h"
#include "core/primitives/FixedVector.h"
#include "slr_config.h"

namespace slr {

using MidiBuffer = FixedVector<MidiEvent, MIDI_BUFFER_DEFAULT_SIZE>;
using MidiBufferRecord = FixedVector<MidiEvent, MIDI_BUFFER_RECORD_SIZE>;

}