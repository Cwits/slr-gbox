// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/ClipItem.h"
#include <vector>
#include <memory>
#include <atomic>

namespace slr {

class File;
// class ClipItem;
struct AudioPeaks;
struct MidiTrack; 

struct ClipContentView {
    ClipContentType type;
    
    bool isMidi() const { return type == ClipContentType::Midi; }
    bool isAudio() const { return type == ClipContentType::Audio; }
    
    union {
        const AudioPeaks * audio;
        const MidiTrack * midi; //TODO: need to replace this by something... dunno
    };
    ID fileId;
};

struct ClipItemView {
    ClipItemView(const ClipItem * container, const ClipContentView &cont);

    frame_t startPosition() const;
    frame_t endPosition() const;
    frame_t length() const;
    frame_t fileOffset() const;
    bool muted() const;

    ID id() const;
    ID fileId() const;

    const ClipContentView _content;

    void update();
    uint64_t version() const { return _version.load(std::memory_order_acquire); }


    // void update(frame_t startPosition, frame_t length, frame_t fileOffset, bool muted) {
    //     _startPosition = startPosition;
    //     _length = length;
    //     _fileOffset = fileOffset;
    //     _muted = muted;
    //     incrementVersion();
    // }

    private:
    // frame_t _startPosition;
    // frame_t _length;
    // frame_t _fileOffset;
    // bool _muted;
    const ClipItem * _item;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
};

struct ClipContainerView {
    ClipContainerView();
    void addClipItem(const ClipItemView *item);
    const std::vector<const ClipItemView*> & clips() const;
    void deleteClipViewItem(ID id);
    
    uint64_t version() const { return _version.load(std::memory_order_acquire); }
    
    private:
    std::vector<const ClipItemView*> _items;
    
    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
};

struct ClipViewStorage {
    ~ClipViewStorage();
    ClipItemView * newClipView(const ClipItem * item, const ClipContentView &cont);
    ClipItemView * findClipById(ID id);
    bool deleteClipById(ID id);

    void updateClips() {
        for(auto & c : _clipList) {
            c->update();
        }
    }
    
    private:
    std::vector<std::unique_ptr<ClipItemView>> _clipList;
};


}