// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/DumpAudio.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>
#include <assert.h>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void DumpAudio::exec(FileWorkerContext &ctx) {
    assert(!file);
    assert(!buffer);

    LOG_INFO("Trying to dump audio data to %s", file->name().c_str());
    bool res = file->dumpRecordedData(buffer);

    finished(res);
}

} //namespace Tasks

} //namespace slr
    