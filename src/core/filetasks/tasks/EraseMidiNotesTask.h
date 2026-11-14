// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/FileTask.h"
#include "core/primitives/MidiPeaks.h"
#include "common/defines.h"

#include <functional>
#include <vector>
#include <memory>

namespace slr {

struct File;
struct FileWorkerContext;
struct MidiFile;
struct MidiTrack;

namespace Tasks {

struct EraseMidiNotes : public FileTask {
    void exec(FileWorkerContext &ctx) override;
    std::vector<Note> notes;
    ID fileId;
    int midiTrackId;

    std::function<void(MidiFile *file, std::unique_ptr<MidiTrack>, bool success)> finished;
};

} //namespace Tasks

} //namespace slr
    