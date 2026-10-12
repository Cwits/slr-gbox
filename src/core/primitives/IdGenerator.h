// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"

namespace slr {

struct IdGenerator {
    IdGenerator();

    ID nextUnitId(ID forced);
    ID nextFileId(ID forced);
    ID nextClipId(ID forced);
    ID nextAudioRouteId(ID forced);
    ID nextMidiRouteId(ID forced);
    ID nextSequenceId(ID forced);
    ID nextModulationId(ID forced);

    private:
    ID _unitIdCounter;
    ID _clipIdCounter;
    ID _fileIdCounter;
    ID _audioRouteIdCounter;
    ID _midiRouteIdCounter;
    ID _sequenceIdCounter;
    ID _modulationIdCounter;
    ID compareAndSetId(const ID & forced, ID &ptr);
};

}