// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/FileContainerView.h"
#include "core/primitives/FileContainer.h"

namespace slr {

ClipItemView::ClipItemView(ClipItem * container) : 
    _startPosition(container->startPosition()),
    _length(container->length()),
    _muted(container->isMuted()),
    _file(container->_file),
    _uniqueId(container->_uniqueId) {

} 
    
}