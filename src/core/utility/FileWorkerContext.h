// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/AudioBuffer.h"

#include <functional>
#include <vector>
#include <memory>
#include <string>

namespace slr {

struct File;
struct FileWorker;
struct IdGenerator;

struct AudioCopyContext {
    std::unique_ptr<AudioBuffer> _data;
    ID originFileId;
    std::string originPath;
};

struct FileWorkerContext {
    FileWorkerContext(
        FileWorker *ptr, 
        std::vector<std::unique_ptr<File>> &files, 
        IdGenerator *gen,
        AudioCopyContext *actx
    );
    ~FileWorkerContext();

    FileWorker * const worker;
    IdGenerator *gen;
    AudioCopyContext * _aCopyCtx;

    File *findFile(ID fileId);
    
    private:
    std::vector<std::unique_ptr<File>> &files;
};

} //namespace slr
    