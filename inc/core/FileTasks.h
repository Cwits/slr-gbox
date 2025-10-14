// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/primitives/Task.h"
#include "core/primitives/AudioBuffer.h"
#include "defines.h"

#include <string>
#include <functional>

namespace slr {

// class AudioBuffer;
class AudioFile;
class File;

namespace Tasks {

/* Generic Tasks */
struct openFile : public Task {
    void exec(FileWorker *f) override;

    std::string path;
    ID targetId;
};

struct closeFile : public Task {
    void exec(FileWorker *f) override;

    File * file;  
};

struct saveFile : public Task {
    void exec(FileWorker *f) override;

    File * file;
};

/* Audio Tasks */
struct dumpAudio : public Task {
    void exec(FileWorker *f) override;

    AudioBuffer * buffer;
    AudioFile * file;
    frame_t size;

    std::function<void(FileWorker*, bool)> callback;
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

struct cutAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct copyAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct pasteAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct deleteAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct silenceAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct reverseAudio : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

struct reverseAudioPhase : public Task {
    void exec(FileWorker *f) override;

    frame_t start;
    frame_t end;
    int channels; //0 - all, otherwise - specified
    ID targetId;
    const AudioFile * file;
};

} //namespace Tasks

} //namespace slr
    