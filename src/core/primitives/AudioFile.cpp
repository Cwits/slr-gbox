// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/utility/helper.h"
#include "logger.h"

#include <unistd.h> //fsync()

namespace slr {

AudioFile::AudioFile() : File(FileType::Audio), _file(nullptr), _info({0}), _data(nullptr), _opened(false), _temporary(false), _finalize(false), _interleave(nullptr), _interleaveChannels(0) {

}

AudioFile::~AudioFile() {
    if(_opened) close();
}

bool AudioFile::createTemporary(std::string & path, int channels, int samplerate) {
    _temporary = true;
    
    _info.channels = channels;
    _info.samplerate = samplerate;
    _info.frames = 0;
#if (SAMPLE_T == float)
    _info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
#elif (SAMPLE_T == double) 
    _info.format = SF_FORMAT_WAV | SF_FORMAT_DOUBLE;
#endif
    _info.sections = 0;

    _path = path;
    return true; //openInternal(path, true);
}

//for files that exists in file system
//otherwise use createTemporary
bool AudioFile::open(std::string & path) {
    return openInternal(path, false);
}

bool AudioFile::save() {
    sf_write_sync(_file);
    clearDirty();

    return true;
}

bool AudioFile::close() {
    
    sf_close(_file);
    _file = nullptr;

    if(!_temporary) {
        for(int i=0; i<_data->channels(); ++i) {
            delete [] (*_data)[i];
        }
        delete [] _data->_data;
        delete _data;
        _data = nullptr;
    }

    _path.erase();
    _name.erase();
    _info = {0};
    _temporary = false;
    _opened = false;
    return true;
}

void AudioFile::prepareForRecord() {
    //???
}

void AudioFile::finishAfterRecord() {
    sf_write_sync(_file);
    
    if(_interleaveChannels > 1) {
        delete [] _interleave;
        _interleaveChannels = 0;
    }
}


bool AudioFile::dumpRecordedData(AudioBuffer * recBuffer) {
    // sample_t * dataPtr = nullptr;
    if(!_opened && _temporary) {
        openInternal(_path, true);
    }

    int channels = recBuffer->channels();
    frame_t size = recBuffer->bufferSize();

    if(channels == 1) {
        _interleave = (*recBuffer)[0];
    } else {
        if(!_interleave) {
            _interleave = new sample_t[size*channels];
            _interleaveChannels = channels;
        }
        packMulti(recBuffer->_data, _interleave, channels, size);
    }
        
    frame_t res = sf_writef_float(_file, _interleave, size);// - for each incomming buffer
    
    if(res != size) {
        LOG_ERROR("Writing to file went wrong!");
        return false;
    }

    _info.frames += size;

    //dump every 8 buffers
    if((_info.frames / size) % 8 == 0) 
        sf_write_sync(_file);
    // if(bufferCounter % 4 == 0)
    //     sf_write_sync() - periodically (either 1-2s or once for several buffers 6-8-10...) and at the end of record

    return true;
}

bool AudioFile::openInternal(std::string & path, bool tmp) {
    if(path.empty()) {
        LOG_ERROR("Path to file empty");
        return false;
    }

    if(!pathHasExtention(Extention::Audio, path)) {
        LOG_ERROR("Wrong file extention");
        return false;
    }

    //check if folders exists

    _file = sf_open(path.c_str(), SFM_READ | SFM_WRITE, &_info);

    if(!_file) {
        std::string errorMsg(sf_error_number(sf_perror(_file)));
        LOG_ERROR("Failed to open file. Error msg: %s", errorMsg.c_str());
        return false;
    }
    
    if(_info.channels > 2) {
        LOG_ERROR("Multichannel audio files not supported");
        sf_close(_file);
        return false;
    }

    if(tmp)  {
        _path = path;
        std::size_t size = path.size();
        std::size_t find = path.find_last_of("/", size);
        _name = std::string(path.substr(find+1, size-find-1));
        _opened = true;
        return true; 
    }

    sample_t * rawPtr = nullptr;
    sample_t ** tmpPtr = nullptr;
    if(!tmp) {
        //read data
        rawPtr = new sample_t[_info.frames*_info.channels];
        if(!rawPtr) {
            LOG_ERROR("Failed to allocate raw buffer");
            sf_close(_file);
            return false;
        }

        

#if (SAMPLE_T == float)
        sf_count_t res = sf_readf_float(_file, rawPtr, _info.frames);
#elif (SAMPLE_T == double)
        sf_count_t res = sf_readf_double(_file, rawPtr, _info.frames);
#endif

        if(res != _info.frames) {
            LOG_ERROR("Failer to read data from file!");
            delete [] rawPtr;
            sf_close(_file);
            return false;
        }

        //audio buffer workaround
        //frame_t size = _info.frames*_info.channels;
        tmpPtr = new sample_t*[_info.channels];
        for(int i=0; i<_info.channels; ++i) tmpPtr[i] = nullptr;

        for(int i=0; i<_info.channels; ++i) {
            sample_t * buffer = static_cast<sample_t*>(new sample_t[_info.frames]);
            if(buffer == nullptr) {
                LOG_ERROR("Failed to allocate buffer for file");
                goto clear;
            }

            clearAudioBuffer(buffer, _info.frames);
            tmpPtr[i] = buffer;
        }

        _data = new AudioBuffer(tmpPtr, _info.channels, _info.frames);
        if(!_data) {
            LOG_ERROR("Failed to allocate AudioBuffer");
            goto clear;
        }

        //unpack data
        unpackMulti(rawPtr, _data->_data, _info.channels, _info.frames);
        
        delete [] rawPtr;

    }

    // std::size_t size = 0;
    // std::size_t find = 0;
    _path = path;
    // size = path.size();
    // find = path.find_last_of("/", size);
    _name = std::string(
        path.substr(
            path.find_last_of("/", path.size())+1,
            path.size()-path.find_last_of("/", path.size())-1)
        );


    _opened = true;
    return true;

    clear:
    for(int i=0; i<_info.channels; ++i) {
        delete [] tmpPtr[i];
    }
    delete [] tmpPtr;
    delete _data;
    delete [] rawPtr;
    sf_close(_file);
    return false;
}   

}