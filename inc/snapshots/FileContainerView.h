// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include <vector>

namespace slr {

class File;
class ContainerItem;

struct ContainerItemView {
    ContainerItemView(ContainerItem * container);
    frame_t _startPosition;
    frame_t _length;
    bool _muted;
    File * _file;
    const ID _uniqueId;
};

class FileContainerView {
    public:

    std::vector<ContainerItemView*> _items;
};


}