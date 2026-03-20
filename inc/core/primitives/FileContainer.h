// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/File.h"
#include "Status.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {

// class AudioUnit;
// class FlatEvents::FlatControl;
// class FlatEvents::FlatResponse;

struct ClipItem {
    ClipItem(const File * const file, frame_t startPos);
    ~ClipItem();

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
    ClipContainer();
    ~ClipContainer();

    bool hasClips() const;
    const std::vector<ClipItem*> * clips() const;

    void appendClip(ClipItem * clip);

    private:
    std::vector<ClipItem*> *_clips;

    friend class AudioUnit;
};

struct ClipStorage {
    ClipStorage();
    std::vector<ClipItem*> * getOtherVector(const std::vector<ClipItem*> * current);

    std::vector<ClipItem*> * getCurrentVector(const std::vector<ClipItem*> * current);

    std::vector<std::unique_ptr<ClipItem>> _clipsOwner;
    
    std::unique_ptr<std::vector<ClipItem*>> _rawVector1; //passing this to RTEngine
    std::unique_ptr<std::vector<ClipItem*>> _rawVector2; //passing this to RTEngine
};

using ClipContainerMap = std::unordered_map<ID, ClipStorage>;

}