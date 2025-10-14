// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/FileWorker.h"

#include "core/primitives/File.h"
#include "core/primitives/AudioFile.h"
#include "core/ControlEngine.h"
#include "logger.h"

#include <iomanip>  //for getDateTime
#include <ctime>    //for getDateTime
#include <sstream>  //for getDateTime
#include <algorithm>

namespace slr {

FileWorker::FileWorker() {

}

FileWorker::~FileWorker() {

}

bool FileWorker::init() {
    for(int i=0; i<8; ++i) {
        std::unique_ptr<AudioFile> file = std::make_unique<AudioFile>();
        _tmpAudioFiles.push_back(std::move(file));
        _usedTmpAudioFiles.push_back(FileAvailability::NotInUse);
    }
    
    _shutdown = false;
    _thread = std::thread(&FileWorker::run, this);

    return true;
}

bool FileWorker::shutdown() {
    _shutdown = true;
    _thread.join();

    // LOG_WARN("Must close all opened files");
    return true;
}

void FileWorker::run(FileWorker * f) {
    while(!f->_shutdown) {
        std::unique_lock<std::mutex> lock(f->_mutex);

        while(f->_queue.empty()) {
            if(f->_cond.wait_for(lock, std::chrono::milliseconds(100)) == std::cv_status::timeout) {
                if(f->_shutdown) {
                    goto exit;
                }
            }
        }

        auto task = std::move(f->_queue.front());
        f->_queue.pop();
        lock.unlock();

        task->exec(f);
    }

    exit:
    LOG_INFO("Exiting File Worker thread");
}

void FileWorker::appendFile(std::unique_ptr<File> file) {
    _fileList.push_back(std::move(file));
}

bool FileWorker::removeFile(File * file) {
    std::size_t id = 0;
    bool found = false;
    std::size_t size = _fileList.size();

    for(std::size_t i=0; i<size; ++i) {
        if(_fileList.at(i).get() == file) {
            found = true;
            id = i;
        }
    }

    if(!found) {
        LOG_ERROR("File not found");
        return false;
    }

    _fileList.erase(_fileList.begin()+id);
    return true;
}

// AudioFile * FileWorker::acquireTmpAudioFile() {
//     return ControlEngine::getInstance()->_fileWorker->acquireTmpAudio();
// }

// void FileWorker::releaseTmpAudioFile(AudioFile * file) {
//     ControlEngine::getInstance()->_fileWorker->releaseTmpAudio(file);
// }

AudioFile * FileWorker::acquireTmpAudioFile() {
    if(_freeTmpFiles == 0) {
        //TODO:tmp file expansion
        //expand tmp files
        if(!expandTmpAudioFile()) {
            LOG_FATAL("Failed to expand tmp audio file storage");
        }
    }

    bool found = false;
    std::size_t id = 0;
    std::size_t size = _usedTmpAudioFiles.size();
    for(std::size_t i=0; i<size; ++i) {
        if(_usedTmpAudioFiles.at(i) == FileAvailability::NotInUse) {
            id = i;
            found = true;
        }
    }

    if(!found) {
        LOG_FATAL("Failed to find new file");
    }

    _usedTmpAudioFiles.at(id) = FileAvailability::InUse;
    _freeTmpFiles--;
    return _tmpAudioFiles.at(id).get();
}

void FileWorker::releaseTmpAudioFile(AudioFile * file) {
    std::size_t size = _usedTmpAudioFiles.size();
    std::size_t id = 0;
    bool found = false;
    for(std::size_t i=0; i<size; ++i) {
        if(_tmpAudioFiles.at(i).get() == file) {
            found = true;
            id = i;
        }
    }

    if(!found) {
        LOG_FATAL("Failed to found audio file");
    }

    _usedTmpAudioFiles.at(id) = FileAvailability::NotInUse;
    _freeTmpFiles++;
}

bool FileWorker::expandTmpAudioFile() {
    try {
        for(int i=0; i<8; ++i) {
            std::unique_ptr<AudioFile> file = std::make_unique<AudioFile>();
            _tmpAudioFiles.push_back(std::move(file));
            _usedTmpAudioFiles.push_back(FileAvailability::NotInUse);
            _freeTmpFiles += 8;
        }
        return true;
    } catch(...) {
        return false;
    }
}

void FileWorker::closeTmpAudioFile(AudioFile * file) {
    try {
        file->save();
        file->close();

        std::size_t idx = 0;
        bool found = false;
        for(std::size_t i=0; i<_tmpAudioFiles.size(); ++i) {
            if(_tmpAudioFiles.at(i).get() == file) {
                found = true;
                idx = i;
                break;
            }
        }

        if(!found) {
            LOG_ERROR("Failed to find such tmp file");
            return;
        }

        _tmpAudioFiles.erase(_tmpAudioFiles.begin()+idx);
        _usedTmpAudioFiles.erase(_usedTmpAudioFiles.begin()+idx);
        _freeTmpFiles--;

        std::unique_ptr<AudioFile> newfile = std::make_unique<AudioFile>();
        _tmpAudioFiles.push_back(std::move(newfile));
        _usedTmpAudioFiles.push_back(FileAvailability::NotInUse);
        _freeTmpFiles++;
    } catch(...) {
        LOG_ERROR("Failed to close tmp file %s", file->name().c_str());
    }
}

const std::vector<File*> FileWorker::listFiles() {
    std::size_t size = _fileList.size();
    std::vector<File*> ret;
    ret.reserve(size);

    for(std::size_t i=0; i<size; ++i) {
        File * fil = _fileList.at(i).get();
        if(fil->isAudio() || fil->isMidi()) {
            //we don't need peak files here
            ret.push_back(_fileList.at(i).get());
        } 
    }

    return ret;
}


//returns string with d-m-Y H-M-S
std::string getDateTime()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y:%H-%M-%S");

    return ss.str();
}

//generate random string with specified length
//len should be > 1
std::string generateRandomName(int length)
{
    char letters[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    char randLetters[length] = {0};

    for(int i=0; i<length-1; ++i)
    {
        randLetters[i] = letters[rand()%26];
    }
    randLetters[length] = '\0';
    
    std::string str;
    str.append(randLetters);

    return str;
}

}