// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/primitives/File.h"
#include "core/primitives/MidiEvent.h"

#include <fstream>

namespace slr {

enum class MidiFileFormat {
    SingleTrack,
    MultipleTrack,
    MultipleSong
};

// class MidiFileView;
struct MidiFile : public File {
    struct MidiTrack {
        //from https://ccrma.stanford.edu/~craig/14q/midifile/MidiFileFormat.html
        //track_event = <v_time> + <midi_event> | <meta_event> | <sysex_event>
        std::vector<MidiEvent> midiEvents;
        std::vector<MetaEvent> metaEvents;
        std::vector<SysexEvent> sysexEvents; 
    };

    MidiFile();
    ~MidiFile();

    bool createTemporary(std::string &path);
    bool open(std::string &path) override;
    bool save() override;
    bool close() override;

    void prepareForRecord() override;
    void finishAfterRecord() override;
    
    const frame_t frames() const override; 

    const bool temporary() const { return _temporary; }
    const bool finalize() const { return _finalize; }
    const bool opened() const { return _opened; }

    void finalizeFile() { _finalize = true; }

    const MidiFileFormat format() const { return _format; }
    const uint32_t ppqn() const { return _ppqn; }
    const std::size_t trackCount() const { return _tracks.size(); }

    const MidiTrack & track(std::size_t id) const { return _tracks.at(id); }

    private:
    std::fstream _handle;

    bool _temporary;
    bool _finalize;
    bool _opened;

    bool openInternal(std::string &path, bool tmp);

    MidiFileFormat _format;
    uint32_t _ppqn;

    std::vector<MidiTrack> _tracks;

    frame_t readvlv(uint8_t * data, int &currentReadPosition);
};
    
}