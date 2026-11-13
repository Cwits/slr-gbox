// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/FileTask.h"
#include "common/defines.h"

#include <functional>
#include <optional>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

struct UpdateAudioPeaks : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    ID audioFileId; //audio file id
    frame_t start;
    frame_t end;
    std::function<void(bool success)> finished;
};

} //namespace Tasks

} //namespace slr
    