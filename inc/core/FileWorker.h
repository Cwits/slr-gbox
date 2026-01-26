// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "core/primitives/Task.h"
#include "core/primitives/File.h"

namespace slr {

class AudioFile;
class MidiFile;

class FileWorker {
    public:
    FileWorker();
    ~FileWorker();

    bool init();
    bool shutdown();

    template<typename T>
    void addTask(std::unique_ptr<T> task) {
        // FileWorkerInst * fw = instance();
        std::lock_guard<std::mutex> lock(_mutex);

        _queue.emplace(std::move(task));
        _cond.notify_one();
    }

    void appendFile(std::unique_ptr<File> file);
    bool removeFile(File * file);
    
    const std::vector<File*> listFiles();  


    AudioFile * acquireTmpAudioFile();
    void releaseTmpAudioFile(AudioFile * file);
    void closeTmpAudioFile(AudioFile * file);
    std::vector<std::unique_ptr<AudioFile>> & tmpAudioFilesList() { 
        return _tmpAudioFiles;
    }
    
    MidiFile * acquireTmpMidiFile();
    void releaseTmpMidiFile(MidiFile *file);
    void closeTmpMidiFile(MidiFile *file);

    private:
    static void run(FileWorker * f);

    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<std::unique_ptr<Task>> _queue;
    std::thread _thread;
    std::atomic<bool> _shutdown;

    std::vector<std::unique_ptr<File>> _fileList;

    bool expandTmpAudioFile();
    enum class FileAvailability { NotInUse = 0, InUse };
    std::vector<std::unique_ptr<AudioFile>> _tmpAudioFiles; 
    std::vector<FileAvailability> _usedTmpAudioFiles;
    std::size_t _freeTmpFiles;
    
    struct TmpMidi {
        bool inUse;
        std::unique_ptr<MidiFile> _file;
    };
    std::vector<TmpMidi> _tmpMidiFiles;
    
    // friend class FileWorker;
};


std::string getDateTime();
std::string generateRandomName(int length);

}