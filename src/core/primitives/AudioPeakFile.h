// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/File.h"
#include "core/primitives/AudioPeaks.h"

#include <array>
#include <cstdio>
#include <cstdint>
#include <fstream>

namespace slr {

class AudioPeakFile : public File {
    public:
    AudioPeakFile(const ID forcedID = 0);
    ~AudioPeakFile();

    bool open(std::string &path) override;
    static bool createAndBuild(const std::string &path, const AudioFile * file);
    bool save() override;
    bool close() override;
    
    void prepareForRecord() override;
    void finishAfterRecord() override;

    bool valid(AudioFile * file);

    AudioPeaks * peaks() { return &_peaks; }
    const AudioPeaks * peaks() const { return &_peaks; }
    
    frame_t frames() const override; 
    
    private:
    
    struct __attribute__((packed)) AudioPeakHeader {
        char magic[5]; //"SLRPK" - 5*8 = 5 bytes
        uint8_t version;        //1 byte
        uint8_t channels;       //1 byte
        uint8_t padding1;       //1 byte
        uint32_t samplerate;    //4 bytes
        uint8_t padding2[4];    //4 bytes
        uint64_t totalSize;     //8 bytes //total size of file including header
        uint8_t reserved[8];    //8 bytes
                                //total 5+1+1+1+4+4+8+8 bytes = 32 bytes
    };

    struct __attribute__((packed)) AudioPeakChunk {
        char magic[4];          //"data" - 4 bytes
        uint8_t LODLevel;       //1 byte
        uint8_t reserved[3];    //3 bytes
        uint64_t chunkSize;     //8 bytes
        // uint8_t * data;      //total 13+3 = 16 bytes
    };

    std::fstream _handle;

    AudioPeaks _peaks;
    
    AudioFile * _relatedAudioFile;
};

}