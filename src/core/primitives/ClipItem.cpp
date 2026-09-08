// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/ClipItem.h"

#include "common/defines.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/File.h"
#include "common/logger.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

namespace slr {

static ID _clipItemUniqueId = 0;

ClipItem::ClipItem(const File * const file, frame_t startPos, long forcedId) : 
    _startPosition(startPos), 
    _length(file->frames()), 
    _fileOffset(0), 
    _muted(false), 
    _file(file), 
    _uniqueId(forcedId == -1 ? _clipItemUniqueId : static_cast<ID>(forcedId)) {

    ID testres = std::max(_uniqueId, _clipItemUniqueId);
    if(testres == _clipItemUniqueId) _clipItemUniqueId = testres+1;
    else _clipItemUniqueId = testres;


}

ClipItem::~ClipItem() {}

// ClipContainerBuffer::ClipContainerBuffer() {
//     _container1 = std::make_unique<ClipContainer>();
//     _container2 = std::make_unique<ClipContainer>();
//     _container1->reserve(2);
//     _container2->reserve(2);

//     // std::unique_ptr<ClipContainer> b1 = std::make_unique<ClipContainer>();
//     // std::unique_ptr<ClipContainer> b2 = std::make_unique<ClipContainer>();
//     // b1->reserve(2); b2->reserve(2);
//     // _containers.init(std::move(b1), std::move(b2));

//     _inUse = false;
// }

// ClipContainer * ClipContainerBuffer::modifiableContainer() {
//     if(!_inUse) return _container1.get();
//     else return _container2.get();
// }

// const ClipContainer * ClipContainerBuffer::inUseContainer() {
//     if(!_inUse) return _container2.get();
//     else return _container1.get();
// }

// void ClipContainerBuffer::clear() {
//     LOG_WARN("Not implemented");
// }

// void ClipContainerBuffer::containerSwapped() {
//     _inUse = !_inUse;
// }

ClipStorage::~ClipStorage() {

}

ClipItem * ClipStorage::newClip(const File * const file, frame_t startPosition, long forcedId) {
    ClipItem * clip = nullptr;
    std::unique_ptr<ClipItem> itemUniq;

    if(forcedId >= 0) itemUniq = std::make_unique<ClipItem>(file, startPosition, forcedId); 
    else itemUniq = std::make_unique<ClipItem>(file, startPosition);
    
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
    
const std::vector<ClipItem*> ClipStorage::items() {
    std::vector<ClipItem*> ret;
    ret.reserve(_clipList.size());

    for(auto & c : _clipList) {
        ret.push_back(c.get());
    }

    return ret;
}

ClipItem * ClipStorage::makeUniqueFrom(const ClipItem *other) {
    LOG_WARN("Not implemented");
    return nullptr;
}

}