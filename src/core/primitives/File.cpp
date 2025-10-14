// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/File.h"

namespace slr {
static ID fileUniqueId = 0;

File::File() 
    : _type(FileType::Error), 
    _uniqueId(fileUniqueId) {
    fileUniqueId++;
}

File::File(FileType type) 
    : _type(type),
    _uniqueId(fileUniqueId) {
    fileUniqueId++;
}

File::~File() {}

}