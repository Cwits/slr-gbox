// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/IdGenerator.h"

namespace slr {

IdGenerator::IdGenerator() {
    _unitIdCounter = 1;
    _clipIdCounter = 1;
    _fileIdCounter = 1;
    _audioRouteIdCounter = 1;
    _midiRouteIdCounter = 1;
    _sequenceIdCounter = 1;
    _modulationIdCounter = 1;
    
}


ID IdGenerator::nextUnitId(ID forced) { return compareAndSetId(forced, _unitIdCounter); }
ID IdGenerator::nextFileId(ID forced) { return compareAndSetId(forced, _fileIdCounter); }
ID IdGenerator::nextClipId(ID forced) { return compareAndSetId(forced, _clipIdCounter); }
ID IdGenerator::nextAudioRouteId(ID forced) { return compareAndSetId(forced, _audioRouteIdCounter); }
ID IdGenerator::nextMidiRouteId(ID forced) { return compareAndSetId(forced, _midiRouteIdCounter); }
ID IdGenerator::nextSequenceId(ID forced) { return compareAndSetId(forced, _sequenceIdCounter); }
ID IdGenerator::nextModulationId(ID forced) { return compareAndSetId(forced, _modulationIdCounter); }

ID IdGenerator::compareAndSetId(const ID & forced, ID &ptr) {
    ID ret = 0;
    if(forced == 0) {
        ret = ptr++;
    } else {
        //check what is bigger
        if(forced > ptr) ptr = forced;
        ret = forced;
    }
    return ret;
}

}