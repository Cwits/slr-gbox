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

// class AudioBuffer;
class AudioFile;
class File;
struct FileWorker;
struct FileWorkerContext;


namespace Tasks {

/* Generic Tasks */
//open, callback, close
struct OneShotOpen : public FileTask {
    /* like for e.g. preset loading - you don't need keep file with preset always opened? */
    void exec(FileWorkerContext &ctx) override {}
};

struct openFile : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    std::string path;
    // ID targetId;
    // frame_t fileStartPosition;
    std::optional<ID> forcedId;
    std::function<void(const File * file, bool success)> finished;
};

struct closeFile : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    File * file;  
};

struct saveFile : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    ID fileId;
};

struct closeAllFiles : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    bool saveFiles;
    std::function<void()> completed;
};

/* Audio FileTasks */
struct dumpAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    AudioBuffer * buffer;
    AudioFile * file;
    frame_t size;

    std::function<void(FileWorker*const, bool)> callback;
    //check AudioFile.cpp for more detailed info
};

/* for any of next audio manipulation tasks:
    if user want to manipulate audio:
    1. from ui issued event with request
    2. Control Engine issue task for FileWorker to manipulate audio
    3. FileWorker clone the file and make changes in clone(add to fileList), not touching original
    4. When done - FileWorker issue event that work is done, sending back 
        pointers to original file and to new file and target id
    5. Control engine pass that information to rt engine 
    6. RT Engine swaps old file and new file in places and issue event that files swapped
        with old file pointer and new file pointer
    7. Control engine pass that back to file worker + update snapshot
    8. File worker can safely close/delete old file
    
    !Note: all these changes doesn't affect actual file on disk. File on disk get's affected 
            only if user save changes, otherwise it's happening only in ram*/
// Cut, Copy, Paste, Delete, Silence, Reverse, ReversePhase };

struct cutAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct copyAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct pasteAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct deleteAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct silenceAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct reverseAudio : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct reverseAudioPhase : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

} //namespace Tasks

} //namespace slr
    