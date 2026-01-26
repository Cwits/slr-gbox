// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace slr {

#define RT_TRACE 0
#define MIDI_TRACE 0

/* Audio Buffer Manager */
#define DEFAULT_BUFFER_COUNT 1024
#define DEFAULT_RECORD_BUFFER_COUNT 16
#define DEFAULT_RECORD_BUFFER_MUL (100*5) //make sure this buffer is big enough...
#define DEFAULT_BUFFER_CHANNELS 2

#define RECORD_QUEUE_LENGTH 64

constexpr std::size_t MIDI_BUFFER_DEFAULT_SIZE = 128;
constexpr std::size_t MIDI_BUFFER_RECORD_SIZE = 0xFFFF;

constexpr int REGULAR_BUFFER_COUNT = 1024;
constexpr int RECORD_BUFFER_COUNT = 128; //assume 4 per track == 32 tracks recording simultaniously
constexpr int RECORD_BUFFER_MUL = 512;
}