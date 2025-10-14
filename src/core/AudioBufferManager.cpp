// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AudioBufferManager.h"
#include "logger.h"
#include "core/primitives/SPSCQueue.h"
#include "core/primitives/AudioBuffer.h"
#include "slr_config.h"

namespace slr {

namespace AudioBufferManager {

int _blockSize = 0;
int _channels = 0;

struct BufferUsage {
    AudioBuffer * _buffer;
    bool _inUse;
};

std::vector<BufferUsage> _regularPool;
std::size_t _totalRegularCount;
std::size_t _freeRegularCount;

std::vector<AudioBuffer*> _recordPool;  
std::size_t _totalRecordCount;

SPSCQueue<AudioBuffer*, RECORD_QUEUE_LENGTH> _recordQueue;
std::size_t _recordQueueActual = 0;
std::atomic<bool> _recordAcquireNotify = false;

/*
TODO: there is possibility that one buffer at the end is not used(when expanding accures)
*/

bool init(int blockSize, int channels) {
    _regularPool.clear();
    _recordPool.clear();
    // _recordQueue.clear();
    _totalRegularCount = 0;
    _freeRegularCount = 0;
    _totalRecordCount = 0;
    _recordQueueActual = 0;
    _recordAcquireNotify = false;

    _blockSize = blockSize;
    _channels = channels;
    
    bool res = false;
    res |= expandRegularPool();
    res |= expandRecordPool();

    if(!res) {
        LOG_FATAL("Failed to init Audio Buffer MAnager");
    }

    LOG_INFO("Audio Buffers allocated successfully");
    return true;
}

bool shutdown() {
     for(auto &b : _regularPool) {
        delete b._buffer;
    }

    for(AudioBuffer * buf : _recordPool) {
        delete buf;
    }

    return true;
}

AudioBuffer * acquireRegular() {
    if(_freeRegularCount == 0) {
        if(!expandRegularPool()) {
            //TODO: Prohibit creating new tracks/fx/whatever using audio buffers if run out of memory
            LOG_ERROR("Ooopsy, run out of memory for Audio Buffers!");
        }
    }
    std::size_t id = 0;
    bool foundFree = false;
    for(std::size_t i=0; i<_totalRegularCount; ++i) {
        BufferUsage & b = _regularPool.at(i);
        if(!b._inUse) {
            b._inUse = true;
            _freeRegularCount--;
            return b._buffer;
        }
    }

    //TODO: Prohibit creating new tracks/fx/whatever using audio buffers if run out of memory
    LOG_FATAL("Couldn't find free buffers");

    return nullptr;
}

void releaseRegular(AudioBuffer * b) {
    if(b == nullptr) { LOG_ERROR("nullptr"); return; }

    std::size_t id = 0;
    bool found = false;
    for(std::size_t i=0; i<_totalRegularCount; ++i) {
        BufferUsage &buse = _regularPool.at(i);
        if(buse._buffer == b) {
            if(buse._inUse == false) {
                LOG_ERROR("Double buffer release");
            }
            buse._inUse = false;
            _freeRegularCount++;
            return;
        }
    }

    LOG_FATAL("Unable to find audio buffer to delete");
}

//TODO: now expansion appears in control thread - that's good, but it happens when new track created
//  it should happen when free buffers running low(like 4-8-12 left), but not when there is no buffers
bool expandRegularPool() {
    //for DEFAULT_BUFFER_COUNT/2
    std::size_t addedCount = (DEFAULT_BUFFER_COUNT/2);
    std::size_t newCount = _totalRegularCount + addedCount;
    try {
        _regularPool.reserve(newCount);

        for(std::size_t i=0; i<addedCount; ++i) {
            AudioBuffer * buf = new AudioBuffer(_blockSize, _channels);
            _regularPool.push_back({buf, false});
        }
    } catch (...) {
        LOG_ERROR("Failed to expand Audio Buffer Pool");
        return false;
    }

    _totalRegularCount += addedCount;
    _freeRegularCount += addedCount;
    return true;
}

AudioBuffer * acquireRecord() {
    // if(_freeRecordBufferCount == 0) {
    //     if(!expandRecordPool()) {
    //         LOG_ERROR("Ooopsy, run out of memory for Audio Buffers!");
    //     }
    // }

    // std::size_t id = 0;
    // bool foundFree = false;
    // for(std::size_t i=0; i<_totalRecordBufferCount; ++i) {
    //     if(_recordBuffersInUse.at(i) == NotInUse) {
    //         foundFree = true;
    //         id = i;
    //         break;
    //     }
    // }

    // if(!foundFree) {
    //     LOG_FATAL("Couldn't find free record buffers");
    // }

    // _recordBuffersInUse.at(id) = InUse;
    // _freeRecordBufferCount--;
    // return _recordBufferPool.at(id);
    AudioBuffer * buffer = nullptr;

    if(!_recordQueue.pop(buffer)) {
        LOG_FATAL("No record buffers available");
    }

    if(_recordAcquireNotify) {
        //if pool is bigger than queue you have to notify through rt responses that buffer acquired
        //so CE can add free buffer
        LOG_WARN("Provide mechanism to notify!");
    }

    return buffer;
}

void releaseRecord(AudioBuffer * b) {
    // if(b == nullptr) { LOG_ERROR("nullptr"); return; }

    // std::size_t id = 0;
    // bool found = false;
    // for(std::size_t i=0; i<_totalRecordBufferCount; ++i) {
    //     if(_recordBufferPool.at(i) == b) {
    //         id = i;
    //         found = true;
    //         break;
    //     }
    // }

    // if(!found) {
    //     LOG_FATAL("Unable to find audio buffer to delete");
    //     return;
    // }
    
    // if(_recordBuffersInUse.at(id) == NotInUse) {
    //     LOG_WARN("Double buffer release");
    // } 

    // _recordBuffersInUse.at(id) = NotInUse;
    // _freeRecordBufferCount++;
    _recordQueue.push(b);
}

bool expandRecordPool() {
    //for DEFAULT_BUFFER_COUNT/2
    std::size_t addedCount = (DEFAULT_RECORD_BUFFER_COUNT/2);
    std::size_t newCount = _totalRecordCount + addedCount;
    try {
        _recordPool.reserve(newCount);
        for(std::size_t i=0; i<addedCount; ++i) {
            AudioBuffer * buf = new AudioBuffer(_blockSize*DEFAULT_RECORD_BUFFER_MUL, _channels);
            _recordPool.push_back(buf);
            if(_recordQueueActual < RECORD_QUEUE_LENGTH) {
                _recordQueue.push(buf);
                _recordQueueActual++;
            }
        }
        _totalRecordCount += addedCount;

        if(_totalRecordCount >= RECORD_QUEUE_LENGTH) {
            _recordAcquireNotify = true;
        }
        
    } catch (...) {
        LOG_ERROR("Failed to expand Audio Buffer Pool");
        return false;
    }

    return true;
}

const int freeRegularCount() { return _freeRegularCount; }
const int totalRegularCount() { return _totalRegularCount; }
const int totalRecordCount() { return _totalRecordCount; } 

}//namespace AudioBufferManager
}//namespace slr