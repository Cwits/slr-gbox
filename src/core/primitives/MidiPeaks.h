// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"

#include <vector>

namespace slr {

struct Note {
    frame_t ppqnStart;
    frame_t ppqnEnd;
    int velocity;
    int note;
    int channel;
};

struct CC {
    frame_t position;
    int value;
};

struct MidiTrack;

struct MidiPeaks {
    void buildPeaks(const MidiTrack*, std::size_t);

    std::size_t trackNo() const { return _trackNo; }
    const std::vector<Note> & notes() const { return _notes; }
    const std::vector<CC> & automations() const { return _automations; }

    private:
    std::size_t _trackNo = -1;
    std::vector<Note> _notes;
    std::vector<CC> _automations;

};


}