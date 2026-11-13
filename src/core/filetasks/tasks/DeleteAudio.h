// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/FileTask.h"
#include "core/primitives/AudioBuffer.h"
#include "common/defines.h"

#include <functional>
#include <optional>

namespace slr {

struct File;
struct FileWorkerContext;
struct AudioFile;

namespace Tasks {

struct DeleteAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    std::optional<int> channel;
    ID fileId;

    std::function<void(AudioFile *target, std::unique_ptr<AudioBuffer> buf, bool success)> finished;
};

} //namespace Tasks

} //namespace slr
    