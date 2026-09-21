// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioPeaks.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"

#include "common/logger.h"

#include <cmath>

namespace slr {

int samplesInLevelByIdx(int idx) {
    int ret = 1;
    switch(idx) {
        case(0): ret = static_cast<int>(LODLevels::LODLevel0); break;
        case(1): ret = static_cast<int>(LODLevels::LODLevel1); break;
        case(2): ret = static_cast<int>(LODLevels::LODLevel2); break;
        case(3): ret = static_cast<int>(LODLevels::LODLevel3); break;
        case(4): ret = static_cast<int>(LODLevels::LODLevel4); break;
        case(5): ret = static_cast<int>(LODLevels::LODLevel5); break;
        case(6): ret = static_cast<int>(LODLevels::LODLevel6); break;
    }

    return ret;
}

uint8_t mapSample(sample_t in) {
    return (in + 1.f) * (255) / 2.f;
}

AudioPeaks::AudioPeaks() {
}

AudioPeaks::~AudioPeaks() {
  
}

void AudioPeaks::build(const AudioFile * file) {
    frame_t size = file->frames();
    int channels = file->channels();
    for(int i=0; i<TOTAL_LOD_LEVELS; ++i) {
        if(i == 0) {
            _peaks0 = std::make_unique<PeakData0>(size, channels);
            uint8_t **data = _peaks0->rawAccesor();
            const sample_t * const* datain = file->data()->rawAccesor();

            for(frame_t f=0; f<size; ++f) {
                for(int ch=0; ch<channels; ++ch) {
                    data[ch][f] = mapSample(datain[ch][f]); 
                }
            }
        } else {
            int idx = i-1;
            int samplesPerFrame = samplesInLevelByIdx(i);
            frame_t newSize = size / samplesPerFrame;
            //TODO: need somehow handle tail
            // frame_t diff = size - (newSize*samplesPerFrame);

            _peaks[idx] = std::make_unique<PeakData>(newSize, channels);
            PeakData * peakData = _peaks[idx].get();

            PeakDataBase ** data = peakData->rawAccesor();
            const sample_t * const* datain = file->data()->rawAccesor();

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
        }
    }
}

void AudioPeaks::extend(const AudioBuffer * buf) {

}

void AudioPeaks::update(const AudioFile * file) {

}

void AudioPeaks::updateRegion(const AudioFile * file, frame_t start, frame_t end) {

}


PeakData * AudioPeaks::peaks(LODLevels lvl) {
    return getPeaks(numFromLevel(lvl));
}
const PeakData * AudioPeaks::peaks(LODLevels lvl) const {
    return getPeaks(numFromLevel(lvl));
}

PeakData * AudioPeaks::peaks(int lvl) {
    return getPeaks(lvl);
}
const PeakData * AudioPeaks::peaks(int lvl) const {
    return getPeaks(lvl);
}

PeakData * AudioPeaks::getPeaks(int lvl) const {
    PeakData * ret = nullptr;
    if(lvl == 0) { LOG_ERROR("For LODLevel 0 use peaks0"); }
    else { ret = _peaks[lvl-1].get(); }
    return ret;
}


LODLevels pickLevel(float ratio) {
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

int numFromLevel(LODLevels lvl) {
    int ret = 0;
    switch(lvl) {
        case(LODLevels::LODLevel0): ret = 0; break;
        case(LODLevels::LODLevel1): ret = 0; break;
        case(LODLevels::LODLevel2): ret = 1; break;
        case(LODLevels::LODLevel3): ret = 2; break;
        case(LODLevels::LODLevel4): ret = 3; break;
        case(LODLevels::LODLevel5): ret = 4; break;
        case(LODLevels::LODLevel6): ret = 5; break;
    }
    return ret;
}

}