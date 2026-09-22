// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

#include "core/primitives/File.h"

namespace slr {

struct AudioFile;
struct MidiFile;
struct FileTask;

class FileWorker {
    public:
    FileWorker();
    ~FileWorker();

    bool init(std::atomic<bool> &shutdown);
    bool shutdown();
    bool clear();

    template<typename T>
    void addTask(std::unique_ptr<T> task) {
        // FileWorkerInst * fw = instance();
        std::lock_guard<std::mutex> lock(_mutex);

        _queue.emplace(std::move(task));
        _cond.notify_one();
    }

    void appendFile(std::unique_ptr<File> file, bool asTemporary = false);
    std::unique_ptr<File> removeFile(File * file, bool asTemporary = false);
    const std::vector<File*> listFiles(FileType type, bool temporary = false) const;
    std::vector<File*> listFiles(FileType type, bool temporary = false);

    private:
    static void run(FileWorker * f, std::atomic<bool> &shutdown);

    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<std::unique_ptr<FileTask>> _queue;
    std::thread _thread;
    std::atomic<bool> _shutdown;

    std::vector<std::unique_ptr<File>> _fileList;
    std::vector<std::unique_ptr<File>> _tmpFileList;

};


}