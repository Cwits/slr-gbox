// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/OpenFile.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/MidiFile.h"
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
            LOG_FAIL("Path or file %s don't exists", path.c_str());
            finished(nullptr, false);
            return;
        }

        std::unique_ptr<AudioFile> ufile;
        if(forcedId) ufile = std::make_unique<AudioFile>(forcedId.value());
        else ufile = std::make_unique<AudioFile>();

        AudioFile * afile = ufile.get();

        if(!afile->open(path)) {
            LOG_FAIL("Unable to open file %s", path.c_str());
            finished(nullptr, false);
            return;
        }
        
        std::unique_ptr<AudioPeakFile> apkfile;
        if(viewForcedId) apkfile = std::make_unique<AudioPeakFile>(viewForcedId.value());
        else apkfile = std::make_unique<AudioPeakFile>();
        
        std::string subpath = afile->path();
        if(!Common::FileIO::changeExtentionTo(subpath, ".slrpk")) {
            //something went off
            LOG_FAIL("Unable to change extention of %s to %s",
                subpath.c_str(), ".slrpk");
            finished(nullptr, false);
            return;
        }

        if(!Common::FileIO::pathOrFileExists(subpath)) {
            //such file don't exists, let's build
            if(!AudioPeakFile::createAndBuild(subpath, afile)) {
                LOG_FAIL("Building peaks for %s went wrong", afile->path().c_str());
                finished(nullptr, false);
                return;
            }
        }

        if(!apkfile->open(subpath)) {
            LOG_FAIL("Failed to open peak file for %s", afile->path().c_str());
            finished(nullptr, false);
            return;
        }

        afile->setPeaks(apkfile->peaks());

        //if total success
        ctx.worker->appendFile(std::move(ufile), false);
        ctx.worker->appendFile(std::move(apkfile), false);
        finished(afile, true);
    } else if(Common::FileIO::pathHasExtention(Common::FileIO::Extention::Midi, path)) {
        if(!Common::FileIO::pathOrFileExists(path)) {
            LOG_FAIL("Path or file %s don't exists", path.c_str());
            finished(nullptr, false);
            return;
        }

        std::unique_ptr<MidiFile> ufile;
        if(forcedId) ufile = std::make_unique<MidiFile>(forcedId.value());
        else ufile = std::make_unique<MidiFile>();

        MidiFile *mfile = ufile.get();

        if(!mfile->open(path)) {
            LOG_FAIL("Unable to open midi file %s", path.c_str());
            finished(nullptr, false);
            return;
        }

        //some kind of midi peak file?

        ctx.worker->appendFile(std::move(ufile), false);
        finished(mfile, true);
    }
}

} //namespace Tasks

} //namespace slr
    