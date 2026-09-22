// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/File.h"
#include "core/primitives/ClipContainer.h"
#include "core/primitives/DoubleBuffer.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {
class Timeline;

struct ClipItem {
    ClipItem(const Timeline *tl, const File * const file, frame_t startPos, long forcedId = -1);
    ~ClipItem();

    frame_t startPosition() const { return _startPosition; }
    frame_t endPosition() const { return _startPosition + _length; }
    frame_t length() const { return _length; }
    frame_t fileOffset() const { return _fileOffset; }
    bool isMuted() const { return _muted; }
    ID id() const { return _uniqueId; }

    const File * const _file;

    void recalculate(const double &coef) {
        /*recalculate 
            for midi items - start and length 
            for audio... start (and length when there will be playback rate introduced)
            meaning that all the files must be recalculated already?
        */

        /*
            в общем просто так не получится, тогда надо как то сопостовлять?
            то есть 
            new position = old position * (new frames per quater note / old frames per quater note);


        */

       if(_file->isMidi()) {
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
    const ID _uniqueId;
};

struct ClipStorage {
    ~ClipStorage();
    ClipItem * newClip(const Timeline *tl, const File *const file, frame_t startPosition, long forcedId = -1);
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