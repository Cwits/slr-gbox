// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioFile.h"
#include "core/utility/basicAudioManipulation.h"
#include "common/FileIO.h"
#include "common/logger.h"
#include <string>
#include <cstdio>
#include <cstdint>
#include <cstring>

namespace slr {

LODLevels levelFromInt(uint8_t lvl) {
    LODLevels level;
    switch(lvl) {
        case(0): level = LODLevels::LODLevel0; break;
        case(1): level = LODLevels::LODLevel1; break;
        case(2): level = LODLevels::LODLevel2; break;
        case(3): level = LODLevels::LODLevel3; break;
        case(4): level = LODLevels::LODLevel4; break;
        case(5): level = LODLevels::LODLevel5; break;
        case(6): level = LODLevels::LODLevel6; break;
    }
    return level;
}

AudioPeakFile::AudioPeakFile(const ID forcedID) :
    File(FileType::AudioPeak, forcedID)
{
}

AudioPeakFile::~AudioPeakFile() {
    close();
}

bool AudioPeakFile::open(std::string &path) {
    if(path.empty()) return false;
    if(!Common::FileIO::pathHasExtention(Common::FileIO::Extention::AudioPeak, path)) return false;
    if(_handle.is_open()) {
        LOG_ERROR("File already opened");
        return false;
    }

    _handle.open(path.c_str(), std::ios::binary | std::ios::in);
    if(!_handle.is_open()) {
        LOG_ERROR("Failed to open file");
        return false;
    }

    AudioPeakHeader _header;
    _handle.read((char*)&_header, sizeof(AudioPeakHeader));
    if(_header.magic[0] != 'S' ||
        _header.magic[1] != 'L' ||
        _header.magic[2] != 'R' ||
        _header.magic[3] != 'P' ||
        _header.magic[4] != 'K') 
    {
        LOG_ERROR("Wrong Audio Peak File magic");
        return false;
    }

    AudioPeakChunk chunk;
    {
        _handle.read((char*)&chunk, sizeof(AudioPeakChunk));
        if(chunk.magic[0] != 'd' ||
            chunk.magic[1] != 'a' ||
            chunk.magic[2] != 't' ||
            chunk.magic[3] != 'a') 
        {
            LOG_ERROR("Wrong chunk format");
            return false;
        }

        //load lod0
        int channels = _header.channels;
        frame_t size = chunk.chunkSize;
        _peaks._peaks0 = std::make_unique<PeakData0>(size, channels);
        PeakData0 * data = _peaks._peaks0.get();
        if(!data) {
            LOG_ERROR("No memory? Error not handled");
            // return false;
        }

        if(_header.channels == 1) {
            _handle.read((char*)data->rawAccesor()[0], chunk.chunkSize);
        } else {
            frame_t total = chunk.chunkSize * _header.channels;
            auto uninterleaved = std::make_unique<uint8_t[]>(total);
            _handle.read((char*)uninterleaved.get(), total);

            unpackMulti(uninterleaved.get(), data->rawAccesor(), _header.channels, chunk.chunkSize);
        }
    }

    {
        //load other lods
        for(int i=1; i<TOTAL_LOD_LEVELS; ++i) {
            int idx = i-1;

            _handle.read((char*)&chunk, sizeof(AudioPeakChunk));
            if(chunk.magic[0] != 'd' ||
                chunk.magic[1] != 'a' ||
                chunk.magic[2] != 't' ||
                chunk.magic[3] != 'a') 
            {
                LOG_ERROR("Wrong chunk format");
                return false;
            }

            int channels = _header.channels;
            frame_t size = chunk.chunkSize;
            _peaks._peaks[idx] = std::make_unique<PeakData>(size, channels);
            PeakData * buffer = _peaks._peaks[idx].get();
            if(!buffer) {
                LOG_ERROR("No memory? Error not handled");
                // return false;
            }

            if(chunk.chunkSize == 0) continue;

            if(_header.channels == 1) {
                _handle.read((char*)buffer->rawAccesor()[0], chunk.chunkSize);
            } else {
                frame_t total = chunk.chunkSize*_header.channels;
                auto uninterleaved = std::make_unique<PeakDataBase[]>(total);

                _handle.read((char*)uninterleaved.get(), total*sizeof(PeakDataBase));

                unpackMulti(uninterleaved.get(), buffer->rawAccesor(), _header.channels, chunk.chunkSize);
            }
        }
    }


    return true;
}

bool AudioPeakFile::createAndBuild(const std::string &path, const AudioFile * file) {
    if(path.empty()) return false;
    if(!Common::FileIO::pathHasExtention(Common::FileIO::Extention::AudioPeak, path)) return false;
    
    std::unique_ptr<AudioPeakFile> tmp = std::make_unique<AudioPeakFile>();

    // Common::FileIO::pathIsValid(path, true);
    if(tmp->_handle.is_open()) {
        LOG_ERROR("Already opened");
        return false;
    }
    
    tmp->_handle.open(path, std::ios_base::binary | std::ios_base::trunc);
    if(!tmp->_handle.is_open()) {
        LOG_ERROR("Failed to open");
        return false;
    }

    AudioPeakHeader header;
    header.magic[0] = 'S';
    header.magic[1] = 'L';
    header.magic[2] = 'R';
    header.magic[3] = 'P';
    header.magic[4] = 'K';
    header.version = 1;
    header.channels = file->channels();
    header.samplerate = file->samplerate();
    header.totalSize = 32;  //no chunks yet
    //clear padding as well
    header.padding1 = 0;
    for(int i=0; i<4; ++i) header.padding2[i] = 0;
    for(int i=0; i<8; ++i) {
        header.reserved[i] = 0;
    }

    tmp->_handle.write((char*)&header, sizeof(AudioPeakHeader));
    
    tmp->_peaks.build(file);
    
    AudioPeakChunk chunk;
    chunk.magic[0] = 'd';
    chunk.magic[1] = 'a';
    chunk.magic[2] = 't';
    chunk.magic[3] = 'a';
    chunk.reserved[0] = 0;
    chunk.reserved[1] = 0;
    chunk.reserved[2] = 0;

    {
        //write lod0
        PeakData0 * buffer = tmp->_peaks._peaks0.get();
        frame_t size = buffer->size();
        int channels = buffer->channels();

        std::unique_ptr<uint8_t[]> holder;
        uint8_t * data = nullptr;
        if(channels == 1) {
            data = (*buffer)[0];
        } else {
            frame_t total = size*channels;
            holder = std::make_unique<uint8_t[]>(total);
            data = holder.get();
            std::memset(data, 0, total*sizeof(uint8_t));

            frame_t flocal = 0;
            for(frame_t f=0; f<total; f+=channels) {
                for(int ch=0; ch<channels; ++ch) {
                    data[f+ch] = (*buffer)[ch][flocal];
                }
                flocal++;
            }
            //data to be deleted later? 
        }

        chunk.LODLevel = 0;
        chunk.chunkSize = size;

        tmp->_handle.write((char*)&chunk, sizeof(AudioPeakChunk));
        tmp->_handle.write((char*)data, size*channels);
        header.totalSize += (size+16);
    }

    {
        //write other LOD's
        for(int i=1; i<TOTAL_LOD_LEVELS; ++i) {
            int idx = i-1;
            PeakData * buffer = tmp->_peaks._peaks[idx].get();

            frame_t size = buffer->size();
            int channels = buffer->channels();

            std::unique_ptr<PeakDataBase[]> holder;
            PeakDataBase * data;
            if(channels == 1) {
                data = (*buffer)[0];
            } else {
                //interleave
                frame_t total = size*channels;
                holder = std::make_unique<PeakDataBase[]>(total);
                data = holder.get();
                frame_t flocal = 0;
                for(frame_t f=0; f<total; f+=channels) {
                    for(int ch=0; ch<channels; ++ch) {
                        data[f+ch] = (*buffer)[ch][flocal];
                    }
                    flocal++;
                }
            }

            chunk.LODLevel = i+1;
            chunk.chunkSize = size;

            tmp->_handle.write((char*)&chunk, sizeof(AudioPeakChunk));
            tmp->_handle.write((char*)data, size*channels*sizeof(PeakDataBase));
            
            header.totalSize += (size+16);
        }
    }

    tmp->_handle.seekp(16);
    tmp->_handle.write((char*)&header.totalSize, 8);

    tmp->_handle.close();

    return true;
}

bool AudioPeakFile::save() {
    //??
    return false;
}

bool AudioPeakFile::close() {
    // fclose(_file);
    if(dirty()) {
        save();
    }
    _handle.close();
    return true;
}

void AudioPeakFile::prepareForRecord() {

}

void AudioPeakFile::finishAfterRecord() {

}

bool AudioPeakFile::valid(AudioFile * file) {
    //check if channels == file channels
    //samplerate == file samplerate
    return true;
}

frame_t AudioPeakFile::frames() const {
    return 0;
}


}