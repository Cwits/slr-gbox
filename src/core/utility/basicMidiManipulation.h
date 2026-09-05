// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/MidiBuffer.h"
#include "core/primitives/MidiEvent.h"

namespace slr {

void sortEventsInMidiBuffer(MidiBuffer *buf);
void copyMidiBuffer(const MidiBuffer *src, MidiBuffer *dst);

}