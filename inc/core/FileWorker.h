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

    AudioFile * acquireTmpAudioFile();
    void releaseTmpAudioFile(AudioFile * file);
    
    void closeTmpAudioFile(AudioFile * file);

    const std::vector<File*> listFiles();  

    std::vector<std::unique_ptr<AudioFile>> & tmpAudioFilesList() { 
        return _tmpAudioFiles;
    }
    //static MidiFile * acquireTmpMidiFile();
    //static MidiFile * releaseTmpMidiFile(MidiFile * file);

    private:
    static void run(FileWorker * f);

    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<std::unique_ptr<Task>> _queue;
    std::thread _thread;

    std::vector<std::unique_ptr<File>> _fileList;

    // AudioFile * acquireTmpAudio();
    // void releaseTmpAudio(AudioFile * file);
    bool expandTmpAudioFile();
    enum class FileAvailability { NotInUse = 0, InUse };
    std::vector<std::unique_ptr<AudioFile>> _tmpAudioFiles;
    std::vector<FileAvailability> _usedTmpAudioFiles;
    std::size_t _freeTmpFiles;
    //std::vector<std::unique_ptr<MidiFile>> _tmpMidiFiles;
    std::atomic<bool> _shutdown;

    // friend class FileWorker;
};


std::string getDateTime();
std::string generateRandomName(int length);

}