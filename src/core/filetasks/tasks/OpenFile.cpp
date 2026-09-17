// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/tasks/OpenFile.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void OpenFile::exec(FileWorkerContext &ctx) {
    if(Common::FileIO::pathHasExtention(Common::FileIO::Extention::Audio, path)) {
        if(!Common::FileIO::pathOrFileExists(path)) {
            LOG_ERROR("Path or file %s don't exists", path.c_str());
            finished(nullptr, false);
            return;
        }
        if(!Common::FileIO::pathHasExtention(Common::FileIO::Extention::Audio, path)) {
            finished(nullptr, false);
            return;
        }

        std::unique_ptr<AudioFile> ufile;
        if(forcedId) ufile = std::make_unique<AudioFile>(forcedId.value());
        else ufile = std::make_unique<AudioFile>();

        AudioFile * afile = ufile.get();

        if(!afile->open(path)) {
            finished(nullptr, false);
            return;
        }
        
        std::unique_ptr<AudioPeakFile> apkfile;
        if(viewForcedId) apkfile = std::make_unique<AudioPeakFile>(viewForcedId.value());
        else apkfile = std::make_unique<AudioPeakFile>();
        
        std::string path = afile->path();
        if(!Common::FileIO::changeExtentionTo(path, ".slrpk")) {
            //something went off
            LOG_ERROR("Unable to change extention of %s to %s",
                afile->path().c_str(), ".slrpk");
            finished(nullptr, false);
            return;
        }

        if(!Common::FileIO::pathOrFileExists(path)) {
            //such file don't exists, let's build
            if(!AudioPeakFile::createAndBuild(path, afile)) {
                LOG_ERROR("Building peaks for %s went wrong", afile->path());
                finished(nullptr, false);
                return;
            }
        }

        if(!apkfile->open(path)) {
            LOG_ERROR("Failed to open peak file for %s", afile->path());
            finished(nullptr, false);
            return;
        }

        afile->setPeaks(apkfile->peaks());

        //if total success
        ctx.worker->appendFile(std::move(ufile));
        ctx.worker->appendFile(std::move(apkfile));
        finished(afile, true);
    } else if(Common::FileIO::pathHasExtention(Common::FileIO::Extention::Midi, path)) {

    }
}

} //namespace Tasks

} //namespace slr
    