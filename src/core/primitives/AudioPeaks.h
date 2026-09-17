// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/Buffer.h"

#include <cstdint>
#include <array>

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
const int TOTAL_LOD_LEVELS = 7;
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

using PeakData0 = Buffer<uint8_t>;
using PeakData = Buffer<PeakDataBase>;

struct AudioPeaks {
    AudioPeaks();
    ~AudioPeaks();

    void build(const AudioFile * file);
    void extend(const AudioBuffer * buf);
    void update(const AudioFile * file);
    void updateRegion(const AudioFile * file, frame_t start, frame_t end);

    PeakData0 * peaks0() { return _peaks0.get(); }
    const PeakData0 * peaks0() const { return _peaks0.get(); }

    PeakData * peaks(LODLevels lvl);
    const PeakData * peaks(LODLevels lvl) const;
    PeakData * peaks(int lvl);
    const PeakData * peaks(int lvl) const;
    
    private:
    std::unique_ptr<PeakData0> _peaks0;
    std::array<std::unique_ptr<PeakData>, 6> _peaks;
    
    PeakData * getPeaks(int lvl) const;

    friend class AudioPeakFile;
};

LODLevels pickLevel(float ratio);
int numFromLevel(LODLevels lvl);
}