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
        Classic,
        OneShot,
        Slicer
    };

    SamplerMode _mode;
    bool _adsrEnabled; //apply to slices as well

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
    void asset(const AudioFile * file);

    void clearAllVoices() const;

    private:
    BufferManager * _bufferManager;
    
    AudioBuffer * _preMix;
    AudioBuffer * _mix;

    const AudioFile * _file;
    
    void addVoice(uint8_t velocity, uint8_t pitch, frame_t playbackStart, frame_t playbackEnd, frame_t delayBeforePlayback) const;
    void deleteVoice(uint8_t velocity, uint8_t pitch) const;
    void playbackVoices(const AudioContext &ctx) const;

    struct Voice {
        bool active;
        uint8_t velocity;
        uint8_t pitch;
        frame_t playbackPosition;
        frame_t playbackEnd;
        frame_t delayBeforePlayback;

        void clear() {
            active = false;
            velocity = 0;
            pitch = 0;
            playbackPosition = 0;
            playbackEnd = 0;
            delayBeforePlayback = 0;
        }
    };

    static constexpr uint8_t TOTAL_MAX_VOICES = 32;

    mutable std::array<Voice, TOTAL_MAX_VOICES> _voices;
    mutable uint8_t _activeVoiceCount;

    ParameterInt _rootPitch;
    ParameterInt _maxVoices;
    ParameterFrame _sampleStart;
    ParameterFrame _sampleEnd;

    void processIncomingMidi() const;
};

}