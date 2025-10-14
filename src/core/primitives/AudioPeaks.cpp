// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioPeaks.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"
#include <cmath>

namespace slr {

uint8_t mapSample(sample_t in) {
    return (in + 1.f) * (255) / 2.f;
}

AudioPeaks::AudioPeaks() {
    _lod0Data = nullptr;
    _peaksData = nullptr;
}

AudioPeaks::~AudioPeaks() {
    if(_level == LODLevels::LODLevel0) {
        if(_lod0Data) delete _lod0Data;
    } else {
        if(_peaksData) delete _peaksData;
    }
}

void AudioPeaks::build(AudioFile * file, LODLevels level) {
    frame_t size = file->frames();
    int channels = file->channels();
    // size /= static_cast<int>(level);
    // _dataSize = size;
    switch(level) {
        case(LODLevels::LODLevel0): {
            // _lod0Data = new uint8_t[size*channels];
            // AudioBuffer * dataBuf = file->getData();
            _lod0Data = new PeakData0(size, channels);
            uint8_t ** data = _lod0Data->_data;
            sample_t ** datain = file->getData()->_data;
            for(frame_t f=0; f<size; ++f) {
                for(int ch=0; ch<channels; ++ch) {
                    data[ch][f] = mapSample(datain[ch][f]); 
                }
            }
        } break;
        case(LODLevels::LODLevel1):
        case(LODLevels::LODLevel2):
        case(LODLevels::LODLevel3):
        case(LODLevels::LODLevel4):
        case(LODLevels::LODLevel5):
        case(LODLevels::LODLevel6): {
            int samplesPerFrame = static_cast<int>(level);
            frame_t newSize = size / samplesPerFrame;
            //TODO: need somehow handle tail
            frame_t diff = size - (newSize*samplesPerFrame);

            _peaksData = new PeakData(newSize, channels);
            PeakDataBase ** data = _peaksData->_data;
            sample_t ** datain = file->getData()->_data;
            
            for(int ch=0; ch<channels; ++ch) {
                for(frame_t f=0; f<newSize; ++f) {
                    int offset = samplesPerFrame * f;
                    sample_t min = 1.0f;
                    sample_t max = -1.0f;

                    for(int off=offset; off<offset+samplesPerFrame; ++off) {
                        max = std::max(max, datain[ch][off]);
                        min = std::min(min, datain[ch][off]);
                    }

                    data[ch][f] = PeakDataBase {
                        .min = mapSample(min),
                        .max = mapSample(max)
                    };
                }
            }
        } break;
    }
    _level = level;
}

void AudioPeaks::extend(AudioBuffer * buf) {

}

void AudioPeaks::update(AudioFile * file) {

}

void AudioPeaks::updateRegion(AudioFile * file, frame_t start, frame_t end) {

}

AudioPeaks::LODLevels AudioPeaks::pickLevel(float ratio) {
    LODLevels ret;
    if(ratio <= static_cast<float>(LODLevels::LODLevel0)) {
        ret = LODLevels::LODLevel0;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel0) && ratio <= static_cast<float>(LODLevels::LODLevel1)) {
        ret = LODLevels::LODLevel1;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel1) && ratio <= static_cast<float>(LODLevels::LODLevel2)) {
        ret = LODLevels::LODLevel2;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel2) && ratio <= static_cast<float>(LODLevels::LODLevel3)) {
        ret = LODLevels::LODLevel3;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel3) && ratio <= static_cast<float>(LODLevels::LODLevel4)) {
        ret = LODLevels::LODLevel4;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel4) && ratio <= static_cast<float>(LODLevels::LODLevel5)) {
        ret = LODLevels::LODLevel5;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel5) && ratio <= static_cast<float>(LODLevels::LODLevel6)) {
        ret = LODLevels::LODLevel6;
    } else if(ratio > static_cast<float>(LODLevels::LODLevel6)) {
        ret = LODLevels::LODLevel6;
    }

    return ret;
}

}