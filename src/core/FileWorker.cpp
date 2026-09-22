// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/FileWorker.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/MidiFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/filetasks/FileTasks.h"
#include "common/logger.h"

#include <algorithm>

namespace slr {

FileWorker::FileWorker() {

}

FileWorker::~FileWorker() {

}

bool FileWorker::init(std::atomic<bool> &shutdown) {
    // for(int i=0; i<8; ++i) {
    //     std::unique_ptr<AudioFile> file = std::make_unique<AudioFile>();
    //     _tmpAudioFiles.push_back(std::move(file));
    //     _usedTmpAudioFiles.push_back(FileAvailability::NotInUse);
    // }
    
    _shutdown = false;
    _thread = std::thread(&FileWorker::run, this, std::reference_wrapper(shutdown));

    return true;
}

bool FileWorker::shutdown() {
    _shutdown = true;
    _thread.join();

    // LOG_WARN("Must close all opened files");
    return true;
}

bool FileWorker::clear() {
    // LOG_FATAL("Not implemented");
    std::atomic<bool> finished;
    finished.store(false);
    auto close = std::make_unique<Tasks::closeAllFiles>();
    close->saveFiles = false;
    close->completed = [&finished]() {
        finished.store(true, std::memory_order_release);
    };
    addTask(std::move(close));


    LOG_ERROR("Must implement temporary files cleaning as well!");
    while( !(finished.load(std::memory_order_acquire) == true)  ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return true;
}

void FileWorker::run(FileWorker * f, std::atomic<bool> &shutdown) {
    while(!shutdown) {
        std::unique_lock<std::mutex> lock(f->_mutex);

        while(f->_queue.empty()) {
            if(f->_cond.wait_for(lock, std::chrono::milliseconds(100)) == std::cv_status::timeout) {
                if(shutdown) {
                    goto exit;
                }
            }
        }

        auto task = std::move(f->_queue.front());
        f->_queue.pop();
        lock.unlock();

        FileWorkerContext ctx(f, f->_fileList);

        task->exec(ctx);
    }

    exit:
    LOG_INFO("Exiting File Worker thread");
}

void FileWorker::appendFile(std::unique_ptr<File> file, bool asTemporary) {
    if(!asTemporary) _fileList.push_back(std::move(file));
    else _tmpFileList.push_back(std::move(file));
}

std::unique_ptr<File> FileWorker::removeFile(File * file, bool asTemporary) {
    if(!file) {
        LOG_ERROR("Invalid pointer");
        return std::unique_ptr<File>();
    }

    std::vector<std::unique_ptr<File>> &list = (!asTemporary) ? _fileList : _tmpFileList;

    auto found = std::find_if(
        list.begin(),
        list.end(),
        [file](const std::unique_ptr<File> &f) {
            return f.get() == file;
        }
    );

    if(found == list.end()) {
        LOG_FAIL("Failed to find such file %s", file->path().c_str());
        return std::unique_ptr<File>();
    }

    std::unique_ptr<File> ret = std::move(*found);
    list.erase(found);

    return ret;
}

const std::vector<File*> FileWorker::listFiles(FileType type, bool temporary) const {
    const std::vector<std::unique_ptr<File>> &list = (!temporary) ? _fileList : _tmpFileList;
    std::vector<File*> ret;
    ret.reserve(list.size());

    if(type == FileType::All) {
        for(auto &f : list) ret.push_back(f.get());
    } else {
        for(auto &f : list) {
            if(f->type() != type) continue;
            
            ret.push_back(f.get());
        }
    }

    return ret;
}

std::vector<File*> FileWorker::listFiles(FileType type, bool temporary) {
    std::vector<std::unique_ptr<File>> &list = (!temporary) ? _fileList : _tmpFileList;
    std::vector<File*> ret;
    ret.reserve(list.size());

    if(type == FileType::All) {
        for(auto &f : list) ret.push_back(f.get());
    } else {
        for(auto &f : list) {
            if(f->type() != type) continue;
            
            ret.push_back(f.get());
        }
    }

    return ret;
}

}