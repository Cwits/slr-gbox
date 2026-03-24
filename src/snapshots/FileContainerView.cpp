// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/FileContainerView.h"
#include "core/primitives/FileContainer.h"

#include "logger.h"

#include <algorithm>

namespace slr {

ClipItemView::ClipItemView(ClipItem * container) : 
    _startPosition(container->startPosition()),
    _length(container->length()),
    _fileOffset(container->fileOffset()),
    _muted(container->isMuted()),
    // _file(container->_file),
    _item(container),
    _uniqueId(container->id()) {

} 

void ClipItemView::update() {
    /* 
        chatgpt says that this is bullshit and i should, at least, use double buffer with atomic pointer to actual data
        otherwise ui can read inconsistent data...
    */
    _startPosition = _item->startPosition();
    _length = _item->length();
    _fileOffset = _item->fileOffset();
    _muted = _item->isMuted();
    incrementVersion();
}

void ClipContainerView::addClipItem(const ClipItemView *item) {
    _items.push_back(item);
    incrementVersion();
}

const std::vector<const ClipItemView*> & ClipContainerView::clips() const { 
    return _items; 
}

void ClipContainerView::deleteClipViewItem(ID id) {
    std::size_t init = _items.size();

    _items.erase(std::remove_if(
        _items.begin(),
        _items.end(),
        [id](const ClipItemView *i) {
            return i->id() == id;
        }
    ), _items.end());

    if(init > _items.size())
        incrementVersion();
    else 
        LOG_ERROR("Failed to delete Clip View Item with id %lu", id);
}

ClipViewStorage::~ClipViewStorage() { }

ClipItemView * ClipViewStorage::newClipView(ClipItem *item) {
    ClipItemView *ret = nullptr;
    try {
        std::unique_ptr<ClipItemView> itm = std::make_unique<ClipItemView>(item);
        ret = itm.get();
        _clipList.push_back(std::move(itm));
    } catch(...) {
        LOG_ERROR("Failed to create ClipItemView");
    }

    return ret;
}

ClipItemView * ClipViewStorage::findClipById(ID id) {
    ClipItemView * ret = nullptr;

    auto found = std::find_if(
        _clipList.begin(),
        _clipList.end(),
        [id](const std::unique_ptr<ClipItemView> &v) {
            return v->id() == id;
        });

    if(found != _clipList.end()) ret = found->get();
    
    return ret;
}

bool ClipViewStorage::deleteClipById(ID id) {
    std::size_t init = _clipList.size(); 

    _clipList.erase(std::remove_if(
        _clipList.begin(),
        _clipList.end(),
        [id](const std::unique_ptr<ClipItemView> &i) {
            return i->id() == id;
        }
    ), _clipList.end());

    return (init != _clipList.size());
}
    
}