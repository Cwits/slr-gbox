// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioFile.h"
#include "core/utility/helper.h"
#include "logger.h"
#include <string>
#include <cstdio>
#include <cstdint>

namespace slr {

AudioPeaks::LODLevels levelFromInt(uint8_t lvl) {
    AudioPeaks::LODLevels level;
    switch(lvl) {
        case(0): level = AudioPeaks::LODLevels::LODLevel0; break;
        case(1): level = AudioPeaks::LODLevels::LODLevel1; break;
        case(2): level = AudioPeaks::LODLevels::LODLevel2; break;
        case(3): level = AudioPeaks::LODLevels::LODLevel3; break;
        case(4): level = AudioPeaks::LODLevels::LODLevel4; break;
        case(5): level = AudioPeaks::LODLevels::LODLevel5; break;
        case(6): level = AudioPeaks::LODLevels::LODLevel6; break;
    }
    return level;
}

AudioPeakFile::AudioPeakFile() {
    _header = {0};
}

AudioPeakFile::~AudioPeakFile() {
    if(_file) close();
}

bool AudioPeakFile::open(std::string &path) {
    //TODO: should handle if user defined separate folder for peak files
    //now assuming that .slrpk stored near audio file

    if(path.empty()) return false;
    if(!pathHasExtention(Extention::AudioPeak, path)) return false;

    // _file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    _file = fopen(path.c_str(), "rb+");
    if(!_file) {
        LOG_ERROR("Failed to open slrpk file at: %s", path.c_str());
        return false;
    }

    fread(&_header, 1, sizeof(AudioPeakHeader), _file);

    if(_header.magic[0] != 'S' || _header.magic[1] != 'L' || 
        _header.magic[2] != 'R' || _header.magic[3] != 'P' ||
        _header.magic[4] != 'K') {
            LOG_ERROR("Wrong Audio Peak File Format");
            // fclose(_file);
            return false;
    }

    //read chunks:
    AudioPeakChunk chunk;
    fread(&chunk, 1, sizeof(AudioPeakChunk), _file);
    if(chunk.magic[0] != 'd' || chunk.magic[1] != 'a' ||
        chunk.magic[2] != 't' || chunk.magic[3] != 'a') {
        LOG_ERROR("Wrong slrpk chunk format");
        // fclose(_file);
        return false;
    }
    
    {
        _lod0Peaks._level = levelFromInt(chunk.LODLevel);
        AudioPeaks::PeakData0 * data0 = new AudioPeaks::PeakData0(chunk.chunkSize, _header.channels);
        
        uint8_t ** dataPtr = data0->_data;
        if(_header.channels == 1) {
            fread(dataPtr[0], 1, chunk.chunkSize, _file);
        } else {
            frame_t total = chunk.chunkSize * _header.channels;
            uint8_t * uninterleaved = new uint8_t[total];
            fread(uninterleaved, 1, total, _file);

            frame_t flocal = 0;
            for(frame_t f=0; f<total; f+=_header.channels) {
                for(int ch=0; ch<_header.channels; ++ch) {
                    dataPtr[ch][flocal] = uninterleaved[f+ch];
                }
                flocal++;
            }

            delete [] uninterleaved;
        }

        _lod0Peaks._lod0Data = data0;
    }

    for(int i=0; i<6; ++i) {
        AudioPeakChunk chunk;
        fread(&chunk, 1, sizeof(AudioPeakChunk), _file);
        if(chunk.magic[0] != 'd' || chunk.magic[1] != 'a' ||
            chunk.magic[2] != 't' || chunk.magic[3] != 'a') {
            LOG_ERROR("Wrong slrpk chunk format");
            // fclose(_file);
            goto exit;
        }

        AudioPeaks::PeakData * buffer = new AudioPeaks::PeakData(chunk.chunkSize, _header.channels);
        AudioPeaks::PeakDataBase ** dataPtr =  buffer->_data;

        if(chunk.chunkSize == 0) continue;
        
        if(_header.channels == 1) {
            fread(dataPtr[0], 2, chunk.chunkSize, _file);
        } else {
            frame_t total = chunk.chunkSize*_header.channels;
            AudioPeaks::PeakDataBase * uninterleaved = new AudioPeaks::PeakDataBase[total];
            fread(uninterleaved, 2, total, _file);

            frame_t flocal = 0;
            for(frame_t f=0; f<total; f+=_header.channels) {
                for(int ch=0; ch<_header.channels; ++ch) {
                    dataPtr[ch][flocal] = uninterleaved[f+ch];
                }
                flocal++;
            }
        }

        _peaks[i]._peaksData = buffer;
        _peaks[i]._level = levelFromInt(chunk.LODLevel);
    }

    return true;

    exit:
    if(_lod0Peaks._lod0Data) delete _lod0Peaks._lod0Data;
    for(int i=0; i<6; ++i) {
        if(_peaks[i]._peaksData) delete _peaks[i]._peaksData;
    }
    return false;
}

//open, build, close
bool AudioPeakFile::createAndBuild(std::string &path, AudioFile * file) {
    if(path.empty()) return false;
    if(!pathHasExtention(Extention::AudioPeak, path)) return false;

    _file = fopen(path.c_str(), "wb+"); //create new
    if(!_file) {
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

    //write header...
    fwrite(&header, 1, sizeof(AudioPeakHeader), _file); //version

    //blahblah
    _lod0Peaks.build(file, AudioPeaks::LODLevels::LODLevel0);

    for(int i=0; i<6; ++i) {
        AudioPeaks::LODLevels lvl;
        switch(i) {
            case(0): lvl = AudioPeaks::LODLevels::LODLevel1; break;
            case(1): lvl = AudioPeaks::LODLevels::LODLevel2; break;
            case(2): lvl = AudioPeaks::LODLevels::LODLevel3; break;
            case(3): lvl = AudioPeaks::LODLevels::LODLevel4; break;
            case(4): lvl = AudioPeaks::LODLevels::LODLevel5; break;
            case(5): lvl = AudioPeaks::LODLevels::LODLevel6; break;
        };
        
        _peaks[i].build(file, lvl);
    }

    //write peaks to file
    uint8_t * data = nullptr;
    AudioPeakChunk chunk;
    chunk.magic[0] = 'd';
    chunk.magic[1] = 'a';
    chunk.magic[2] = 't';
    chunk.magic[3] = 'a';
    chunk.reserved[0] = 0;
    chunk.reserved[1] = 0;
    chunk.reserved[2] = 0;

    {//write LODLevel 0 peaks
        AudioPeaks::PeakData0 * buffer = _lod0Peaks.lod0Data();
        frame_t size = buffer->bufferSize();
        int channels = buffer->channels();
        //interleave
        if(channels == 1) {
            data = (*buffer)[0];
        } else {
            // interleave data...
            frame_t totalSize = size*channels;
            data = new uint8_t[size*channels];
            std::memset(data, 0, size*channels*sizeof(uint8_t));
            
            frame_t flocal = 0;
            for(frame_t f=0; f<totalSize; f+=channels) {
                for(int ch=0; ch<channels; ++ch) {
                    data[f+ch] = (*buffer)[ch][flocal];
                }
                flocal++;
            }
        }

        chunk.LODLevel = 0;
        chunk.chunkSize = size;

        //write peak level 0
        fwrite(&chunk, 1, sizeof(AudioPeakChunk), _file);
        fwrite(data, 1, size*channels, _file);
        header.totalSize += (size+16); //+ chunk size

        if(channels != 1) delete [] data;
    }

    //write other peak levels
    for(int i=0; i<6; ++i) {
        AudioPeaks::PeakData * buffer = _peaks[i].data();
        frame_t size = buffer->bufferSize();
        int channels = buffer->channels();

        AudioPeaks::PeakDataBase * data;
        if(channels == 1) {
            data = (*buffer)[0];
        } else {
            //interleave
            frame_t total = size*channels;
            data = new AudioPeaks::PeakDataBase[total];
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

        //write peak level 0
        fwrite(&chunk, 1, sizeof(AudioPeakChunk), _file);
        // fwrite(data, 1, size*channels*2, _file); //*2 because we have min and max
        fwrite(data, 2, size*channels, _file); //*2 because we have min and max
        
        header.totalSize += (size+16); //+ chunk size

        //header.totalSize += (size+16);
        if(channels != 1) delete [] data;
    }

    fseek(_file, 16, SEEK_SET);
    fwrite(&header.totalSize, 1, 8, _file);


    //close
    fclose(_file);
    _file = nullptr;

    return true;
}

bool AudioPeakFile::save() {

    return false;
}

bool AudioPeakFile::close() {
    fclose(_file);
    return true;
}

void AudioPeakFile::prepareForRecord() {

}

void AudioPeakFile::finishAfterRecord() {

}


bool AudioPeakFile::build(AudioFile * file) {

    return false;
}

bool AudioPeakFile::exists(std::string & path) {
    //TODO: should handle if user defined separate folder for peak files
    //now assuming that .slrpk stored near audio file
    if(!pathHasExtention(Extention::AudioPeak, path)) {
        LOG_ERROR("Wrong Audio Peak File extention");
        return false;
    }

    FILE* f = std::fopen(path.c_str(), "r");
    if(f) {
        std::fclose(f);
        return true;
    }
    return false;
};

bool AudioPeakFile::valid(AudioFile * file) {
    //check if channels == file channels
    //samplerate == file samplerate
    return true;
}

const AudioPeaks::PeakData * AudioPeakFile::data(AudioPeaks::LODLevels level) const {
    const AudioPeaks::PeakData * ret;
    switch(level) {
        case(AudioPeaks::LODLevels::LODLevel0): ret = _peaks[0].data(); LOG_ERROR("For peak 0 level call data0()!!"); break;
        case(AudioPeaks::LODLevels::LODLevel1): ret = _peaks[0].data(); break;
        case(AudioPeaks::LODLevels::LODLevel2): ret = _peaks[1].data(); break;
        case(AudioPeaks::LODLevels::LODLevel3): ret = _peaks[2].data(); break;
        case(AudioPeaks::LODLevels::LODLevel4): ret = _peaks[3].data(); break;
        case(AudioPeaks::LODLevels::LODLevel5): ret = _peaks[4].data(); break;
        case(AudioPeaks::LODLevels::LODLevel6): ret = _peaks[5].data(); break;
    }
    return ret;
}

AudioPeaks::PeakData * AudioPeakFile::data(AudioPeaks::LODLevels level) {
    AudioPeaks::PeakData * ret;
    switch(level) {
        case(AudioPeaks::LODLevels::LODLevel0): ret = _peaks[0].data(); LOG_ERROR("For peak 0 level call data0()!!"); break;
        case(AudioPeaks::LODLevels::LODLevel1): ret = _peaks[0].data(); break;
        case(AudioPeaks::LODLevels::LODLevel2): ret = _peaks[1].data(); break;
        case(AudioPeaks::LODLevels::LODLevel3): ret = _peaks[2].data(); break;
        case(AudioPeaks::LODLevels::LODLevel4): ret = _peaks[3].data(); break;
        case(AudioPeaks::LODLevels::LODLevel5): ret = _peaks[4].data(); break;
        case(AudioPeaks::LODLevels::LODLevel6): ret = _peaks[5].data(); break;
    }
    return ret;
}

const AudioPeaks::PeakData0 * AudioPeakFile::data0() const {
    return _lod0Peaks.lod0Data();
}

AudioPeaks::PeakData0 * AudioPeakFile::data0() {
    return _lod0Peaks.lod0Data();
}

const frame_t AudioPeakFile::frames() const {
    return 0;
}


}