// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"

#include <cstdint>
#include <vector>
#include <array>

namespace slr {

enum class MidiEventType : uint8_t {
  InvalidType           = 0x00, // For notifying errors
  NoteOff               = 0x80,
  NoteOn                = 0x90,
  Aftertouch            = 0xA0,
  CC                    = 0xB0,
  ProgramChange         = 0xC0, // Program Change
  ChannelPressure       = 0xD0,
  PitchBend             = 0xE0,
  SysEx                 = 0xF0,
  TimeCodeQuarterFrame  = 0xF1, // System Common - MIDI Time Code Quarter Frame
  SongPosition          = 0xF2, // System Common - Song Position Pointer
  SongSelect            = 0xF3, // System Common - Song Select
  TuneRequest           = 0xF6, // System Common - Tune Request
  Clock                 = 0xF8, // System Real Time - Timing Clock
  Start                 = 0xFA, // System Real Time - Start
  Continue              = 0xFB, // System Real Time - Continue
  Stop                  = 0xFC, // System Real Time - Stop
  ActiveSensing         = 0xFE, // System Real Time - Active Sensing
  SystemReset           = 0xFF, // System Real Time - System Reset

};

using uchar = unsigned char;
using MidiMessage = std::vector<uchar>;

struct MidiEvent {
    MidiEventType type;
    int channel;
    int note;
    int velocity;
};

struct RtMidiBuffer {
    ID id;
    std::vector<MidiEvent> * buffer;
};

inline MidiEventType getEventType(MidiMessage *msg) { return static_cast<MidiEventType>(*msg->begin() & 0xF0); }

/* note or controller number */
inline uchar getNumber(MidiEventType type, MidiMessage *msg) {
    switch(type) {
        case MidiEventType::NoteOn:
        case MidiEventType::NoteOff:
        case MidiEventType::Aftertouch:
        case MidiEventType::CC:
            return (*msg)[1];
        default:
            return 0;
    }
}

/* velocity/ammount/value */
inline uchar getValue(MidiEventType type, MidiMessage *msg) {
    switch (type) {
        case MidiEventType::NoteOn:
        case MidiEventType::NoteOff:
        case MidiEventType::Aftertouch:
        case MidiEventType::CC:
        case MidiEventType::PitchBend:
            return (*msg)[2];
        case MidiEventType::ChannelPressure:
            return (*msg)[1];
        default:
            return 0;
    }
}

} //namespace slr
