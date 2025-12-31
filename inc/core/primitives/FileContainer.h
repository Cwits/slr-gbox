// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/File.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {

class AudioUnit;

struct ClipItem {
    ClipItem(const File * const file, frame_t startPos) : _startPosition(startPos), _length(file->frames()), _fileOffset(0), _muted(false), _file(file), _uniqueId(file->id()) {}
    ~ClipItem() {}

    frame_t startPosition() const { return _startPosition; }
    frame_t endPosition() const { return _startPosition + _length; }
    frame_t length() const { return _length; }
    frame_t fileOffset() const { return _fileOffset; }
    bool isMuted() const { return _muted; }

    // const File * file() const { return _file; }

    const File * const _file;
    const ID _uniqueId;
    private:
    frame_t _startPosition;
    frame_t _length;
    frame_t _fileOffset;
    bool _muted;

    friend class AudioUnit;
};

//for use in Audio Unit
struct ClipContainer {
    ClipContainer() : _clips(nullptr) {}
    ~ClipContainer() {}

    bool hasClips() const { return !_clips && _clips->size() > 0; }
    const std::vector<ClipItem*> * clips() const { return _clips; }

    void appendClip(ClipItem * clip) { _clips->push_back(clip); }

    private:
    std::vector<ClipItem*> *_clips;

    friend class AudioUnit;
};

struct ClipStorage {
    ClipStorage() {
        _rawVector1 = std::make_unique<std::vector<ClipItem*>>();
        _rawVector2 = std::make_unique<std::vector<ClipItem*>>();
        _rawVector1->reserve(2);
        _rawVector2->reserve(2);
    }
    std::vector<ClipItem*> * getOtherVector(const std::vector<ClipItem*> * current) {
        if(current == nullptr) return _rawVector1.get();
        if(current == _rawVector1.get()) 
            return _rawVector2.get();
        
        return _rawVector1.get();
    }

    std::vector<ClipItem*> * getCurrentVector(const std::vector<ClipItem*> * current) {
        if(current == _rawVector1.get()) return _rawVector1.get();

        return _rawVector2.get();
    }

    std::vector<std::unique_ptr<ClipItem>> _clipsOwner;
    
    std::unique_ptr<std::vector<ClipItem*>> _rawVector1;
    std::unique_ptr<std::vector<ClipItem*>> _rawVector2;
};

using ClipContainerMap = std::unordered_map<ID, ClipStorage>;

}