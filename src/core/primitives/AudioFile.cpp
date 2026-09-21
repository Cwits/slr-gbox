// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"
#include "common/FileIO.h"
#include "common/logger.h"

#include <unistd.h> //fsync()

namespace slr {

AudioFile::AudioFile(const ID forcedId) : 
    File(FileType::Audio, forcedId), 
    _file(nullptr), 
    _info({0}), 
    _data(std::unique_ptr<AudioBuffer>()), 
    _opened(false), 
    _temporary(false), 
    _finalize(false), 
    _interleave(nullptr), 
    _interleaveChannels(0) 
{

}

AudioFile::~AudioFile() {
    if(_opened) close();
    if(_interleave) _interleave.reset();
}

bool AudioFile::prepareAsTemporary(std::string & path, int channels, int samplerate) {
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
    return true;
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
        // delete [] _interleave; // better do this through buffer manager?
        _interleave.reset();
        _interleaveChannels = 0;
        // _interleave = nullptr;
    }
}

bool AudioFile::dumpRecordedData(AudioBuffer * recBuffer) {
    if(!_opened && _temporary) {
        if(!openInternal(_path, true)) {
            LOG_ERROR("Failed to create or open a file %s", _path.c_str());
            return false;
        }
    }

    size_t channels = recBuffer->channels();
    frame_t size = recBuffer->size();

    sample_t *ptr = nullptr;
    if(channels == 1) {
        ptr = (*recBuffer)[0];
    } else {
        if(!_interleave) {
            // _interleave = new sample_t[size*channels];
            _interleave = std::unique_ptr<sample_t[]>(new sample_t[size*channels]{});
            if(!_interleave) return false;
            _interleaveChannels = channels;
        }
        
        packMulti(recBuffer->rawAccesor(), _interleave.get(), channels, size);
        ptr = _interleave.get();
    }
        
    frame_t res = sf_writef_float(_file, ptr, size);
    
    if(res != size) {
        LOG_ERROR("Written %lu and expected to write %lu sizes not equal", res, size);
        return false;
    }

    _info.frames += size;

    //dump every 8 buffers
    if((_info.frames / size) % 8 == 0) 
        sf_write_sync(_file);

    return true;
}

bool AudioFile::openInternal(std::string & path, bool isTemporary) {
    //TODO: check if already opened, and if file exists 
    if(path.empty()) {
        LOG_ERROR("Path to file empty");
        return false;
    }

    if(!Common::FileIO::pathHasExtention(Common::FileIO::Extention::Audio, path)) {
        LOG_ERROR("Wrong file extention");
        return false;
    }

    // Common::FileIO::pathIsValid(path, true);

    _file = sf_open(path.c_str(), SFM_READ | SFM_WRITE, &_info);
    
    if(!_file) {
        std::string errorMsg(sf_error_number(sf_perror(_file)));
        LOG_ERROR("Failed to open file. Error msg: %s", errorMsg.c_str());
        return false;
    }
    
    if(_info.channels > 2) {
        LOG_ERROR("Multichannel audio files not supported yet");
        sf_close(_file);
        return false;
    }

    if(isTemporary)  {
        _path = path;
        std::size_t size = path.size();
        std::size_t find = path.find_last_of("/", size);
        _name = std::string(path.substr(find+1, size-find-1));
        _opened = true;
        return true; 
    }

    if(!readAndUnpack()) {
        close();
        return false;
    }
    
    _path = path;
    _name = std::string(
        path.substr(
            path.find_last_of("/", path.size())+1,
            path.size()-path.find_last_of("/", path.size())-1)
        );


    _opened = true;
    return true;
}   

bool AudioFile::readAndUnpack() {
    std::unique_ptr<sample_t[]> raw = std::unique_ptr<sample_t[]>(new sample_t[_info.frames*_info.channels]{});

    if(!raw) {
        LOG_ERROR("Failed to allocate raw buffer");
        sf_close(_file);
        return false;
    }
    
    if(_data) {
        LOG_WARN("Old audio data to be deleted");
        _data.reset(nullptr);
    }

    _data = std::make_unique<AudioBuffer>(_info.channels, _info.frames);
    if(!_data) {
        LOG_ERROR("Failed to allocate AudioBuffer");
        // delete [] rawPtr;
        sf_close(_file);
        return false;
    }
        

#if (SAMPLE_T == float)
    sf_count_t res = sf_readf_float(_file, raw.get(), _info.frames);
#elif (SAMPLE_T == double)
    sf_count_t res = sf_readf_double(_file, rawPtr, _info.frames);
#endif

    if(res != _info.frames) {
        LOG_ERROR("Failer to read data from file!");
        // delete [] rawPtr;
        sf_close(_file);
        return false;
    }

    unpackMulti(raw.get(), _data->rawAccesor(), _info.channels, _info.frames);

    // delete [] rawPtr;
    return true;
}

}