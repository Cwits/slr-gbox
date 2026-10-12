// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/ClipItem.h"

#include "common/defines.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/File.h"
#include "core/Timeline.h"
#include "common/logger.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

namespace slr {

ClipItem::ClipItem(const Timeline *tl, const ClipContent &content, frame_t startPos, ID selfId) : 
    _startPosition(startPos), 
    _fileOffset(0), 
    _muted(false), 
    _content(std::move(content)), 
    _id(selfId)
{
    if(_content.isMidi()) {
        _length = (_content.midi->midiEvents.back().offset / (float)tl->ppqn()) * tl->framesPerQuater();
    } else {
        _length = static_cast<frame_t>(_content.audio->size());
    }
}

ClipItem::~ClipItem() {}


ClipStorage::~ClipStorage() {

}

ClipItem * ClipStorage::newClip(const Timeline *tl, const ClipContent &content, frame_t startPosition, ID clipId) {
    ClipItem * clip = nullptr;
    std::unique_ptr<ClipItem> itemUniq;

    itemUniq = std::make_unique<ClipItem>(tl, content, startPosition, clipId); 
    
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