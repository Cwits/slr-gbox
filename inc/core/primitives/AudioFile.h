// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/primitives/File.h"
#include "core/primitives/AudioBuffer.h"
#include "defines.h"

#include <sndfile.h>

namespace slr {
class AudioPeakFile;
// class AudioBuffer;
class AudioFile : public File {
    public:
    AudioFile();
    ~AudioFile();

    //temporary meaned for recording only -> creates new file, don't allocate buffer for getData()
    bool createTemporary(std::string & path, int channels, int samplerate);
    bool open(std::string & path) override;
    bool save() override;
    bool close() override;

    void prepareForRecord() override;
    void finishAfterRecord() override;

    const bool temporary() const { return _temporary; }
    const bool finalize() const { return _finalize; }
    const bool opened() const { return _opened; }

    void finalizeFile() { _finalize = true; }

    const frame_t frames() const { return static_cast<frame_t>(_info.frames); };
    const int channels() const { return _info.channels; }
    const int samplerate() const { return _info.samplerate; }
    const int sections() const { return _info.sections; }
    const int format() const { return _info.format; }
    const int seekable() const { return _info.seekable; }

    const AudioBuffer * getData() const { return _data; }
    bool dumpRecordedData(AudioBuffer * recBuffer); //called from file factory

    const AudioPeakFile * peaks() const { return _peaks; }
    void setPeaks(AudioPeakFile * pk) { _peaks = pk; }
    private:
    SNDFILE * _file;
    SF_INFO _info;
    AudioBuffer * _data;
    
    AudioPeakFile * _peaks;
    
    bool _opened;
    bool _temporary;
    bool _finalize;

    //used only for recording multichannel audio
    sample_t * _interleave;
    uint8_t _interleaveChannels;

    bool openInternal(std::string & path, bool tmp);
};
    
}