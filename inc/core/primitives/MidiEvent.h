// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"

#include <cstdint>
#include <vector>
#include <array>
#include <chrono>

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

enum class MetaEventType : uint8_t {
    SequenceNumber                  = 0x00,
    TextEvent                       = 0x01,
    CopyrightNotice                 = 0x02,
    SequenceOrTrackName             = 0x03,
    InstrumentName                  = 0x04,
    LyricText                       = 0x05,
    MarkerText                      = 0x06,
    CuePoint                        = 0x07,
    MIDIChannelPrefixAssignment     = 0x20,
    EndofTrack                      = 0x2F,
    TempoSetting                    = 0x51,
    SMPTEOffset                     = 0x54,
    TimeSignature                   = 0x58,
    KeySignature                    = 0x59,
    SequencerSpecificEvent          = 0x7F
};

// using uchar = unsigned char;
// using MidiMessage = std::vector<uchar>;

struct MidiEvent {
    MidiEventType type;
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
    frame_t offset; //depends on context - in file = from file start, in rt - from current frame start

    MidiEvent & operator=(const MidiEvent &other) {
        type = other.type;
        channel = other.channel;
        note = other.note;
        velocity = other.velocity;
        offset = other.offset;
        return *this;
    }
};

struct MetaEvent {
    MetaEventType type;
    frame_t offset;
    std::vector<uint8_t> data;
};

struct SysexEvent {
    frame_t offset;
    std::vector<uint8_t> data;
};

inline int expectedLength(const MidiEventType &type) {
    switch(type) {
        case(MidiEventType::NoteOff):
        case(MidiEventType::NoteOn):
        case(MidiEventType::Aftertouch):
        case(MidiEventType::CC):
        case(MidiEventType::PitchBend):
        case(MidiEventType::SongPosition): return 2; break;
        case(MidiEventType::ProgramChange):
        case(MidiEventType::ChannelPressure):
        case(MidiEventType::TimeCodeQuarterFrame):
        case(MidiEventType::SongSelect): return 1; break;
        case(MidiEventType::SysEx): return -1; break;
        default: return 0; break;
    }
}

} //namespace slr
