// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/AudioUnit.h"
// #include "core/FlatEvents.h"
// #include "common/Status.h"
#include "common/defines.h"

namespace slr {

class BufferManager;
class AudioFile;

struct Sampler : public AudioUnit {
    enum class SamplerMode {
        ADSR,
        OneShot
    };

    Sampler(const ClipContainer * initContainer, const ID forcedId);
    ~Sampler();
    
    bool create(BufferManager *man);
    bool destroy(BufferManager *man);

    frame_t process(const AudioContext &ctx,  const Dependencies &inputs) const override;

    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;

    // const AudioBuffer * outputs() const { return _postFX; }
    
    const AudioFile * asset() const { return _file; }
    void asset(const AudioFile * file) { _file = file; }
    void clearAllVoices();

    private:
    BufferManager * _bufferManager;
    
    AudioBuffer * _preMix;
    AudioBuffer * _mix;

    const AudioFile * _file;

    void addVoice(uint8_t velocity, uint8_t pitch, frame_t playbackStart, frame_t playbackEnd, frame_t delayBeforePlayback);
    void deleteVoice();

    struct Voice {
        uint8_t velocity;
        uint8_t pitch;
        frame_t playbackPosition;
        frame_t playbackEnd;
        frame_t delayBeforePlayback;

        // Voice& operator=(const Voice &other) {

        //     return *this;
        // }

        void clear() {
            velocity = 0;
            pitch = 0;
            playbackPosition = 0;
            playbackEnd = 0;
            delayBeforePlayback = 0;
        }
    };

    static constexpr uint8_t TOTAL_MAX_VOICES = 32;

    std::array<Voice, TOTAL_MAX_VOICES> _voices;
    uint8_t _activeVoiceCount;
    uint8_t _voicesHead;
    uint8_t _voicesTail;
    uint8_t _maxVoices;
};

}