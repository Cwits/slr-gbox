// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/OpenFile.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiPeaks.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"
#include "core/Project.h"

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

        ID afileId = ctx.gen->nextFileId(forcedId ? forcedId.value() : 0);

        std::unique_ptr<AudioFile> ufile = std::make_unique<AudioFile>(afileId);

        AudioFile * afile = ufile.get();

        if(!afile->open(path)) {
            LOG_FAIL("Unable to open file %s", path.c_str());
            finished(nullptr, false);
            return;
        }
        
        ID pfileId = ctx.gen->nextFileId(viewForcedId ? viewForcedId.value() : 0);

        std::unique_ptr<AudioPeakFile> apkfile = std::make_unique<AudioPeakFile>(pfileId);
        
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
        LOG_SUCCESS("Audio File %s loaded with ID %u, peak file ID %u", 
            afile->path().c_str(), 
            afile->id(),
            apkfile->id()
        );

        ctx.worker->appendFile(std::move(ufile), false);
        ctx.worker->appendFile(std::move(apkfile), false);

        finished(afile, true);
    } else if(Common::FileIO::pathHasExtention(Common::FileIO::Extention::Midi, path)) {
        //midi file
        if(!Common::FileIO::pathOrFileExists(path)) {
            LOG_FAIL("Path or file %s don't exists", path.c_str());
            finished(nullptr, false);
            return;
        }

        ID mfileId = ctx.gen->nextFileId(forcedId ? forcedId.value() : 0);
        std::unique_ptr<MidiFile> ufile = std::make_unique<MidiFile>(mfileId);

        MidiFile *mfile = ufile.get();

        if(!mfile->open(path)) {
            LOG_FAIL("Unable to open midi file %s", path.c_str());
            finished(nullptr, false);
            return;
        }

        std::vector<std::unique_ptr<MidiPeaks>> peakList;

        for(std::size_t trk=0; trk<mfile->trackCount(); ++trk) {
            std::unique_ptr<MidiPeaks> peaks = std::make_unique<MidiPeaks>();
            peaks->buildPeaks(mfile->track(trk), trk);

            peakList.push_back(std::move(peaks));
        }

        mfile->setPeaks(peakList);

        //some kind of midi peak file?
        LOG_SUCCESS("Midi File %s loaded with ID %u", 
            ufile->path().c_str(), 
            ufile->id()
        );
        ctx.worker->appendFile(std::move(ufile), false);
        finished(mfile, true);
    }
}

} //namespace Tasks

} //namespace slr
    