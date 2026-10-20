// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/FileTask.h"
#include "core/primitives/AudioBuffer.h"
#include "common/defines.h"

#include <string>
#include <functional>
#include <optional>

namespace slr {

struct File;
struct FileWorkerContext;
struct AudioFile;

namespace Tasks {

struct DumpAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    AudioBuffer * buffer;
    AudioFile * file;
    frame_t size;
    std::function<void(bool)> finished;
};

} //namespace Tasks

} //namespace slr
    