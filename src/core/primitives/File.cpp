// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/File.h"

#include <cmath>

namespace slr {
static ID fileUniqueId = 0;

File::File() 
    : _type(FileType::Error), 
    _uniqueId(fileUniqueId) {
    fileUniqueId++;
    _offline = false;
    _dirty.store(false, std::memory_order_relaxed);
}

File::File(FileType type, long forcedId) 
    : _type(type),
    _uniqueId(forcedId == -1 ? fileUniqueId : static_cast<ID>(forcedId)) {
    // fileUniqueId++;
    
    ID testres = std::max(_uniqueId, fileUniqueId);
    if(testres == fileUniqueId) fileUniqueId = testres+1;
    else fileUniqueId = testres;

    _offline = false;
    _dirty.store(false);
}

File::~File() {}

}