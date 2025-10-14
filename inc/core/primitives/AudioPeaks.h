// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/Buffer.h"
#include <cstdint>


namespace slr {

class AudioFile;
class AudioPeakFile;

/* 
    //data stored in save way as in wav file:
    //LLLLLL for 1 ch
    //LRLRLRLR for 2 ch
    //in case of level0 it will be LLL or LRLR
    //in other cases: 
    //minmaxminmaxminmaxminmax according to channels
*/

class AudioPeaks {
    public:
    enum class LODLevels {
        LODLevel0 = 1,
        LODLevel1 = 64,
        LODLevel2 = 256,
        LODLevel3 = 1024,
        LODLevel4 = 2048,
        LODLevel5 = 4096,
        LODLevel6 = 8192
    };

    struct PeakDataBase {
        uint8_t min;
        uint8_t max;
    };
    //127 - middle point(0.0f)
    using PeakData0 = Buffer<uint8_t>;
    using PeakData = Buffer<PeakDataBase>;
    
    AudioPeaks();
    ~AudioPeaks();

    void build(AudioFile * file, LODLevels level);
    void extend(AudioBuffer * buf);
    void update(AudioFile * file);
    void updateRegion(AudioFile * file, frame_t start, frame_t end);

    PeakData0 * lod0Data() { return _lod0Data; }
    const PeakData0 * lod0Data() const { return _lod0Data; }
    PeakData * data() { return _peaksData; }
    const PeakData * data() const { return _peaksData; }
    // const frame_t dataSize() const { return _dataSize; }
    const LODLevels level() const { return _level; }

    static LODLevels pickLevel(float ratio);

    private:
    PeakData0 * _lod0Data;
    PeakData * _peaksData; //according to 7 levels of LODLevels
    LODLevels _level;

    friend class AudioPeakFile;
    // frame_t _dataSize;
};

}