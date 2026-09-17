// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/primitives/File.h"
#include "core/primitives/AudioBuffer.h"
#include "common/defines.h"

#include <sndfile.h>

namespace slr {

class AudioPeaks;

struct AudioFile : public File {
    AudioFile(const ID forcedId = 0);
    ~AudioFile();

    //temporary meaned for recording only -> creates new file, don't allocate buffer for getData()
    bool prepareAsTemporary(std::string & path, int channels, int samplerate);
    bool open(std::string & path) override;
    bool save() override;
    bool close() override;

    void prepareForRecord() override;
    void finishAfterRecord() override;
    void finalize() { _finalize = true; }

    bool temporary() const { return _temporary; }
    bool isFinalize() const { return _finalize; }
    bool opened() const { return _opened; }


    frame_t frames() const { return static_cast<frame_t>(_info.frames); };
    int channels() const { return _info.channels; }
    int samplerate() const { return _info.samplerate; }
    int sections() const { return _info.sections; }
    int format() const { return _info.format; }
    int seekable() const { return _info.seekable; }

    const AudioBuffer * data() const { return _data.get(); }
    bool dumpRecordedData(AudioBuffer * recBuffer); //called from file factory

    const AudioPeaks * peaks() const { return _peaks; }
    void setPeaks(const AudioPeaks * pk) { _peaks = pk; }

    private:
    SNDFILE * _file;
    SF_INFO _info;
    std::unique_ptr<AudioBuffer> _data; //interleaved
    
    const AudioPeaks * _peaks;
    
    bool _opened;
    bool _temporary;
    bool _finalize;

    //used only for recording multichannel audio
    std::unique_ptr<sample_t[]> _interleave;
    uint8_t _interleaveChannels;

    bool openInternal(std::string & path, bool tmp);
    bool readAndUnpack();
};
    
}