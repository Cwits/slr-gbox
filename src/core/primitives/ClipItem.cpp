// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/ClipItem.h"

#include "common/defines.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/File.h"
#include "core/Timeline.h"
#include "common/logger.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

namespace slr {

static ID _clipItemUniqueId = 0;

ClipItem::ClipItem(const Timeline *tl, const File * const file, frame_t startPos, long forcedId) : 
    _startPosition(startPos), 
    _length(file->frames()), 
    _fileOffset(0), 
    _muted(false), 
    _file(file), 
    _uniqueId(forcedId == -1 ? _clipItemUniqueId : static_cast<ID>(forcedId)) {

    ID testres = std::max(_uniqueId, _clipItemUniqueId);
    if(testres == _clipItemUniqueId) _clipItemUniqueId = testres+1;
    else _clipItemUniqueId = testres;

    if(file->isMidi()) {
        _length = (file->frames() / tl->ppqn()) * tl->framesPerQuater();
    }
}

ClipItem::~ClipItem() {}


ClipStorage::~ClipStorage() {

}

ClipItem * ClipStorage::newClip(const Timeline *tl, const File * const file, frame_t startPosition, long forcedId) {
    ClipItem * clip = nullptr;
    std::unique_ptr<ClipItem> itemUniq;

    if(forcedId >= 0) itemUniq = std::make_unique<ClipItem>(tl, file, startPosition, forcedId); 
    else itemUniq = std::make_unique<ClipItem>(tl, file, startPosition);
    
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
    
const std::vector<ClipItem*> ClipStorage::items() const {
    std::vector<ClipItem*> ret;
    ret.reserve(_clipList.size());

    for(auto & c : _clipList) {
        ret.push_back(c.get());
    }

    return ret;
}

std::vector<ClipItem*> ClipStorage::items() {
    std::vector<ClipItem*> ret;
    ret.reserve(_clipList.size());
    for(auto &c : _clipList) ret.push_back(c.get());
    return ret;
}

ClipItem * ClipStorage::makeUniqueFrom(const ClipItem *other) {
    LOG_WARN("Not implemented");
    return nullptr;
}

}