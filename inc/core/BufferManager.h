// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "core/primitives/SPSCQueue.h"
#include <vector>
#include <memory>

namespace slr {

struct BufferManager {

    bool init(int blockSize, int channels);
    bool shutdown();

    AudioBuffer* acquireAudioRegular();
    AudioBuffer* acquireAudioRecord();
    bool releaseAudioRegular(AudioBuffer *buf);
    bool releaseAudioRecord(AudioBuffer *buf);

    MidiBuffer* acquireMidiRegular();
    MidiBufferRecord* acquireMidiRecord();
    bool releaseMidiRegular(MidiBuffer *buf);
    bool releaseMidiRecord(MidiBufferRecord *buf);

    private:
    std::vector<std::unique_ptr<AudioBuffer>> _regularAudioHolder;
    std::vector<AudioBuffer*> _regularAudioFree;

    std::vector<std::unique_ptr<AudioBuffer>> _recordAudioHolder;
    SPSCQueue<AudioBuffer*, RECORD_BUFFER_COUNT> _recordAudioQueue;

    std::vector<std::unique_ptr<MidiBuffer>> _regularMidiHolder;
    std::vector<MidiBuffer*> _regularMidiFree;

    std::vector<std::unique_ptr<MidiBufferRecord>> _recordMidiHolder;
    SPSCQueue<MidiBufferRecord*, RECORD_BUFFER_COUNT> _recordMidiQueue;

    uint32_t _blockSize = 0;
    uint32_t _channels = 0;
};

}
