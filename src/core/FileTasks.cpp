// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/FileTasks.h"
#include "core/utility/helper.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/FileWorker.h"
#include "core/Events.h"
#include "Status.h"
#include "logger.h"

#include <memory>

namespace slr {

namespace Tasks {

/* Generic Tasks */
void openFile::exec(FileWorker *f) {
    if(pathHasExtention(Extention::Audio, path)) {
        std::unique_ptr<AudioFile> file = std::make_unique<AudioFile>();

        if(file->open(path)) {
            AudioFile * afile = file.get();
            f->appendFile(std::move(file));

            std::unique_ptr<AudioPeakFile> apk = std::make_unique<AudioPeakFile>();
            bool success = false;
            std::string path = afile->path();
            path = path.substr(0, path.size()-4);
            path.append(".slrpk");

            if(AudioPeakFile::exists(path)) {
                if(apk->open(path)) {
                    success = true;
                }
            } else {
                if(apk->createAndBuild(path, afile)) {
                    if(apk->open(path)) {
                        success = true;
                    }
                }
            }

            if(!apk->valid(afile)) {
                success = false;
            }

            if(success) {
                afile->setPeaks(apk.get());
                f->appendFile(std::move(apk));

                Events::FileOpened e = {
                    .status = slr::Status::Ok,
                    .file = afile,
                    .targetId = targetId
                };

                EmitEvent(e);
                LOG_INFO("File %s loaded to trackid: %d", afile->path().c_str(), targetId);
            }
            //check for existing peaks file, if there is no such - build
            /* pseudo

            //later in gui:
            uint8_t * data = audioFile->getPeaks(LODLevel::Level1);
            frame_t dataSize = audioFile->getPeaks(LODLevel::Level1);
            //draw

            or

            draw = interpolate(dataLevel1, dataSize1, dataLevel2, dataSize2);
            */
        } else {
            LOG_WARN("Failed to open Audio File");
        }

    } else if(pathHasExtention(Extention::Midi, path)) {
        LOG_ERROR("No routine to open midi file");
    }
}

void closeFile::exec(FileWorker *f) {
    if(!file->close()) LOG_WARN("Failed to close file");
    
    if(!f->removeFile(file)) LOG_WARN("Failed to remove file");
}

void saveFile::exec(FileWorker *f) {
    if(!file->save()) LOG_WARN("Failed to save file");
}

/* Audio Tasks */
void dumpAudio::exec(FileWorker *f) {
    //check testFileWorker.cpp for example
    LOG_INFO("Dumping data to file %s", file->name().c_str());
    bool res = file->dumpRecordedData(buffer);
    
    if(!res) {
        //TODO: handle failure
        //e.g. reacquire another buffer to keep writing, or file? or... idk...
        //at least stop the record and notify...
        LOG_ERROR("Dumping data to file %s failed", file->name().c_str());
    }

    //emit event
    callback(f, res);
}

/* Audio Buffer Editing */
void cutAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void copyAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void pasteAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void deleteAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void silenceAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void reverseAudio::exec(FileWorker *f) {
    //file->markDirty();
}

void reverseAudioPhase::exec(FileWorker *f) {
    //file->markDirty();
}



} //namespace Tasks

} //namespace slr