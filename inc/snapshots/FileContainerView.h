// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include <vector>

namespace slr {

class File;
class ClipItem;

struct ClipItemView {
    ClipItemView(ClipItem * container);
    frame_t _startPosition;
    frame_t _length;
    bool _muted;
    const File * const _file;
    const ID _uniqueId;
};

struct ClipContainerView {
    std::vector<ClipItemView*> _items;
    uint64_t _version;
};


}