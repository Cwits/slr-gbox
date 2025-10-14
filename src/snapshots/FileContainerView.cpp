// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/FileContainerView.h"
#include "core/primitives/FileContainer.h"

namespace slr {

ContainerItemView::ContainerItemView(ContainerItem * container) : 
    _startPosition(container->_startPosition),
    _length(container->_length),
    _muted(container->_muted),
    _file(container->_file),
    _uniqueId(container->_uniqueId) {

}

    
}