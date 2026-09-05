// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/filetasks/FileTasks.h"
#include "core/utility/helper.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"
// #include "core/Events.h"
#include "common/logger.h"

#include <memory>
#include <algorithm>

namespace slr {

namespace Tasks {

/* Generic Tasks */
void openFile::exec(FileWorkerContext &ctx) {
    if(pathHasExtention(Extention::Audio, path)) {
        std::unique_ptr<AudioFile> file;
        if(forcedId) file = std::make_unique<AudioFile>(forcedId.value());
        else file = std::make_unique<AudioFile>();

        if(file->open(path)) {
            AudioFile * afile = file.get();
            ctx.worker->appendFile(std::move(file));

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
                ctx.worker->appendFile(std::move(apk));
            }
            finished(afile, success);
            
        } else {
            LOG_WARN("Failed to open Audio File");
        }

    } else if(pathHasExtention(Extention::Midi, path)) {
        LOG_ERROR("No routine to open midi file");
    }
}

void closeFile::exec(FileWorkerContext &ctx) {
    if(!file->close()) LOG_WARN("Failed to close file");
    
    if(!ctx.worker->removeFile(file)) LOG_WARN("Failed to remove file");
}

void saveFile::exec(FileWorkerContext &ctx) {
    // std::vector<File*> list = f->listFiles;
    auto it = std::find_if(
        ctx.files.begin(),
        ctx.files.end(),
        [id = fileId](const std::unique_ptr<File> &f) {
            return f->id() == id;
        }
    );

    if(it == ctx.files.end()) {
        LOG_ERROR("Failed to find file with id %u", fileId);
        return;
    }

    (*it)->save();
}

void closeAllFiles::exec(FileWorkerContext &ctx) {
    // bool saveFiles;
    // std::function<void()> completed;

    for(auto &f : ctx.files) {
        if(saveFiles) f->save();

        f.reset();
    }

    completed();
};

/* Audio Tasks */
void dumpAudio::exec(FileWorkerContext &ctx) {
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
    callback(ctx.worker, res);
}

/* Audio Buffer Editing */
/* 
    need some actions for swaping audiobuffer in file after task.
    e.g. cut audio - buffer size change

    1. calculate diff
    2. create new buffer
    3. fill new buffer with data
    4. swap buffer in file thru Events
        (it will automatically apply for each of clipitems
        because clipitems has pointers)
*/
void cutAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void copyAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void pasteAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void deleteAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void silenceAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void reverseAudio::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}

void reverseAudioPhase::exec(FileWorkerContext &ctx) {
    //file->markDirty();
}



} //namespace Tasks

} //namespace slr