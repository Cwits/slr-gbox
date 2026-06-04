// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Track/TrackActions.h"

// #include "core/primitives/AudioBuffer.h"
#include "core/Actions.h"

#include <memory>  

namespace slr {

// struct AudioFile;

namespace RtTasks {

void DumpAudioFlat::execRT() {
    auto act = std::make_unique<slr::Actions::DumpRecordedAudio>();
    act->targetBuffer = targetBuffer;
    act->targetFile = targetFile;
    act->size = size;
    act->fileStartPosition = fileStartPosition;
    act->trackId = trackId;
    slr::EmitAction(std::move(act));
}


void ReinitTrackFlat::execRT() {
    // Common::Status status;
    // Track * track;
    auto act = std::make_unique<slr::Actions::ReinitTrackRecord>();
    act->track = track;
    slr::EmitAction(std::move(act));
}

};

};

