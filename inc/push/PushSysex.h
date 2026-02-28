// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Color.h"

#include <unordered_set>

namespace PushLib {

class PushMidi;

struct PushSysex {
    PushSysex(PushMidi &midi);
    ~PushSysex();

    void registerCmdWithReply(unsigned char command);

    void handle(SysexMsg &msg);

    private:
    PushMidi &_midi;
    std::unordered_set<unsigned char> _commandsWithReply;

    friend class PushMidi;
};

}