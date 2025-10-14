// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cstdint>

namespace MIDIEvent {

enum class MidiEvent : uint8_t {
  InvalidType           = 0x00, // For notifying errors
  NoteOff               = 0x80,
  NoteOn                = 0x90,
  Aftertouch            = 0xA0,
  CC                    = 0xB0,
  ChannelPressure       = 0xD0,
  PitchBend             = 0xE0,
  ProgramChange         = 0xC0, // Program Change
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


using uint = unsigned int;
using byte = unsigned char;
using midi_msg = std::vector<byte>;

byte get_midi_type(midi_msg &message) { return *message.begin() & 0xF0; }
uint get_midi_number(byte msg_type, midi_msg &message) {
  switch (msg_type) {
  case MidiMsgType::note_on:
  case MidiMsgType::note_off:
  case MidiMsgType::aftertouch:
  case MidiMsgType::cc:
    return message[1];
  default:
    //oopsy
  }
}
uint get_midi_value(byte msg_type, midi_msg &message) {
  switch (msg_type) {
  case MidiMsgType::note_on:
  case MidiMsgType::note_off:
  case MidiMsgType::aftertouch:
  case MidiMsgType::cc:
  case MidiMsgType::pitch_bend:
    return message[2];
  case MidiMsgType::channel_pressure:
    return message[1];
  default:
    //oopsy
  }
}

}