// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include <vector>
#include <memory>
#include <atomic>

namespace slr {

class File;
class ClipItem;

struct ClipItemView {
    ClipItemView(ClipItem * container);

    frame_t startPosition() const { return _startPosition; }
    frame_t endPosition() const { return _startPosition + _length; }
    frame_t length() const { return _length; }
    frame_t fileOffset() const { return _fileOffset; }
    bool muted() const { return _muted; }

    const ClipItem * item() const { return _item; }
    ID id() const { return _uniqueId; }

    void update();
    uint64_t version() const { return _version.load(std::memory_order_acquire); }

    private:
    frame_t _startPosition;
    frame_t _length;
    frame_t _fileOffset;
    bool _muted;
    const ClipItem * const _item;
    const ID _uniqueId;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
};

struct ClipContainerView {
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
    ClipItemView * newClipView(ClipItem * item);
    ClipItemView * findClipById(ID id);
    bool deleteClipById(ID id);
    
    private:
    std::vector<std::unique_ptr<ClipItemView>> _clipList;
};


}