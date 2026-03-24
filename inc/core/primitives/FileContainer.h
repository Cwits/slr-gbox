// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/File.h"
#include "core/primitives/ClipContainer.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {

class Project;

struct ClipItem {
    ClipItem(const File * const file, frame_t startPos);
    ~ClipItem();

    frame_t startPosition() const { return _startPosition; }
    frame_t endPosition() const { return _startPosition + _length; }
    frame_t length() const { return _length; }
    frame_t fileOffset() const { return _fileOffset; }
    bool isMuted() const { return _muted; }
    ID id() const { return _uniqueId; }

    const File * const _file;
    private:
    
    frame_t _startPosition;
    frame_t _length;
    frame_t _fileOffset; //inside of clip
    bool _muted;
    const ID _uniqueId;

    friend class Project;
};

//for use in Audio Unit

struct ClipContainerBuffer {
    ClipContainerBuffer();

    ClipContainer * modifiableContainer();
    const ClipContainer * inUseContainer();
    void clear();

    void containerSwapped();
    
    private:
    bool _inUse;

    std::unique_ptr<ClipContainer> _container1; //passing this to RTEngine
    std::unique_ptr<ClipContainer> _container2; //passing this to RTEngine
};

using ClipContainerMap = std::unordered_map<ID, ClipContainerBuffer>;

struct ClipStorage {
    ~ClipStorage();
    ClipItem * newClip(const File *const file, frame_t startPosition);
    // ClipItem * duplicateFrom(const ClipItem *item);

    ClipItem * findClipById(ID id);
    void deleteClipById(ID id);

    ClipItem * makeUniqueFrom(const ClipItem *other);
    
    private:
    std::vector<std::unique_ptr<ClipItem>> _clipList;
};

}