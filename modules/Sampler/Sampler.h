// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/AudioUnit.h"
// #include "core/FlatEvents.h"
// #include "common/Status.h"
#include "defines.h"

namespace slr {

class BufferManager;
class AudioFile;

struct Sampler : public AudioUnit {
    enum class SamplerMode {
        ADSR,
        OneShot
    };

    Sampler(const ClipContainer * initContainer);
    ~Sampler();
    
    bool create(BufferManager *man);
    bool destroy(BufferManager *man);

    RT_FUNC frame_t process(const AudioContext &ctx,  const Dependencies &inputs) override;

    RT_FUNC void prepareToPlay() override;
    RT_FUNC void prepareToRecord() override;
    RT_FUNC void stopPlaying() override;
    RT_FUNC void stopRecording() override;

    // const AudioBuffer * outputs() const { return _postFX; }
    
    //TODO:: it is incorrect use, now just for test and progress. See TODO2.txt for info
    RT_FUNC static Common::Status assetOpened(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    const AudioFile * asset() const { return _file; }

    private:
    BufferManager * _bufferManager;
    
    AudioBuffer * _preMix;
    AudioBuffer * _mix;

    const AudioFile * _file;

    void addVoice(uint8_t velocity, uint8_t pitch, frame_t playbackStart, frame_t playbackEnd, frame_t delayBeforePlayback);
    void deleteVoice();
    void clearAllVoices();

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