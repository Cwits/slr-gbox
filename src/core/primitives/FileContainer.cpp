// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/FileContainer.h"

#include "defines.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/File.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace slr {

static ID _clipItemUniqueId = 0;

ClipItem::ClipItem(const File * const file, frame_t startPos) : 
    _startPosition(startPos), 
    _length(file->frames()), 
    _fileOffset(0), 
    _muted(false), 
    _file(file), 
    // _uniqueId(_clipItemUniqueId++) {}
    _uniqueId(file->id()) {}

ClipItem::~ClipItem() {}


//for use in Audio Unit
ClipContainer::ClipContainer() : _clips(nullptr) {}
ClipContainer::~ClipContainer() {}

bool ClipContainer::hasClips() const {
    return !_clips && _clips->size() > 0; 
}

const std::vector<ClipItem*> * ClipContainer::clips() const { 
    return _clips; 
}

void ClipContainer::appendClip(ClipItem * clip) { 
    _clips->push_back(clip); 
}


ClipStorage::ClipStorage() {
    _rawVector1 = std::make_unique<std::vector<ClipItem*>>();
    _rawVector2 = std::make_unique<std::vector<ClipItem*>>();
    _rawVector1->reserve(2);
    _rawVector2->reserve(2);
}

std::vector<ClipItem*> * ClipStorage::getOtherVector(const std::vector<ClipItem*> * current) {
    if(current == nullptr) return _rawVector1.get();
    if(current == _rawVector1.get()) 
        return _rawVector2.get();
        
    return _rawVector1.get();
}

std::vector<ClipItem*> * ClipStorage::getCurrentVector(const std::vector<ClipItem*> * current) {
    if(current == _rawVector1.get()) return _rawVector1.get();

    return _rawVector2.get();
}

}