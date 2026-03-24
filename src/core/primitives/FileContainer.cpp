// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/FileContainer.h"

#include "defines.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/File.h"
#include "logger.h"

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
    _uniqueId(_clipItemUniqueId++) {}

ClipItem::~ClipItem() {}

ClipContainerBuffer::ClipContainerBuffer() {
    _container1 = std::make_unique<ClipContainer>();
    _container2 = std::make_unique<ClipContainer>();
    _container1->reserve(2);
    _container2->reserve(2);
    _inUse = false;
}

ClipContainer * ClipContainerBuffer::modifiableContainer() {
    if(!_inUse) return _container1.get();
    else return _container2.get();
}

const ClipContainer * ClipContainerBuffer::inUseContainer() {
    if(!_inUse) return _container2.get();
    else return _container1.get();
}

void ClipContainerBuffer::clear() {
    LOG_WARN("Not implemented");
}

void ClipContainerBuffer::containerSwapped() {
    _inUse = !_inUse;
}

ClipStorage::~ClipStorage() {

}

ClipItem * ClipStorage::newClip(const File * const file, frame_t startPosition) {
    ClipItem * clip = nullptr;
    std::unique_ptr<ClipItem> itemUniq = std::make_unique<ClipItem>(file, startPosition);
    clip = itemUniq.get();

    _clipList.push_back(std::move(itemUniq));
    return clip;
}

ClipItem * ClipStorage::findClipById(ID id) {
    ClipItem * ret = nullptr;

    auto found = std::find_if(
        _clipList.begin(), 
        _clipList.end(),
        [id](std::unique_ptr<ClipItem> &i) {
            return i->id() == id;
        }
    );

    if(found != _clipList.end()) ret = found->get();

    return ret;
}

void ClipStorage::deleteClipById(ID id) {
    LOG_WARN("Not implemented");
}
    
ClipItem * ClipStorage::makeUniqueFrom(const ClipItem *other) {
    LOG_WARN("Not implemented");
    return nullptr;
}

}