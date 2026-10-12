// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/File.h"
#include "core/primitives/ClipContainer.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/DoubleBuffer.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {

class Timeline;
struct MidiTrack;

enum class ClipContentType {
    Audio,
    Midi,
    Automation
};

struct ClipContent {
    ClipContentType type;

    bool isMidi() const { return type == ClipContentType::Midi; }
    bool isAudio() const { return type == ClipContentType::Audio; }

    union {
        const AudioBuffer * audio;
        const MidiTrack * midi;
    };

    ID fileId;
};

struct ClipItem {
    ClipItem(const Timeline *tl, const ClipContent &content, frame_t startPos, ID selfId);
    ~ClipItem();

    frame_t startPosition() const { return _startPosition; }
    frame_t endPosition() const { return _startPosition + _length; }
    frame_t length() const { return _length; }
    frame_t fileOffset() const { return _fileOffset; }
    bool isMuted() const { return _muted; }
    ID id() const { return _id; }

    const ClipContent _content;

    void recalculate(const double &coef) {
       if(_content.isMidi()) {
            _startPosition = _startPosition * coef;
            _length = _length * coef;
       } else {
            _startPosition = _startPosition * coef;
       }
    }
    
    void update(frame_t startPosition, frame_t length, frame_t fileOffset, bool muted) {
        _startPosition = startPosition;
        _length = length;
        _fileOffset = fileOffset;
        _muted = muted;
    }

    private:
    frame_t _startPosition;
    frame_t _length;
    frame_t _fileOffset; //inside of clip
    bool _muted;
    ID _id;
};

struct ClipStorage {
    ~ClipStorage();
    ClipItem * newClip(const Timeline *tl, const ClipContent &content, frame_t startPosition, ID clipId);
    // ClipItem * duplicateFrom(const ClipItem *item);

    ClipItem * findClipById(ID id);
    void deleteClipById(ID id);
    const std::vector<ClipItem*> items() const;
    std::vector<ClipItem*> items();

    ClipItem * makeUniqueFrom(const ClipItem *other);
    
    private:
    std::vector<std::unique_ptr<ClipItem>> _clipList;
};

using ContainerBuffer = DoubleBuffer<std::unique_ptr<ClipContainer>>;
using ClipContainerMap = std::unordered_map<ID, ContainerBuffer>;

}