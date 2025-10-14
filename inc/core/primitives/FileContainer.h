// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/File.h"

#include <vector>

namespace slr {

struct ContainerItem {
    ContainerItem(File * file) : _startPosition(0), _length(0), _muted(false), _file(file), _uniqueId(file->id()) {}
    ~ContainerItem() {}
    frame_t _startPosition;
    frame_t _length;
    bool _muted;
    File * const _file;
    const ID _uniqueId;

    private:
};

class FileContainer {
    public:
    FileContainer() { 
        _items = new std::vector<ContainerItem*>();
        _items->reserve(4); }
    ~FileContainer() { 
        for(std::size_t i=0; i<_items->size(); ++i) {
            delete _items->at(i);
        }
    }

    // RTVector<ContainerItem*> _items;
    std::vector<ContainerItem*> * _items; //prepare new one in Control Engine and swap in rt i guess 
};

}