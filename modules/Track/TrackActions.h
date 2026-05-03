// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/ActionBase.h"
#include "core/primitives/RtTask.h"
#include "core/primitives/AudioBuffer.h"
#include "common/Status.h"
#include "defines.h"

namespace slr {

struct AudioFile;
struct Track;

namespace Actions {

struct DumpRecordedAudio : public ActionBase {
    DumpRecordedAudio() {}
    DumpRecordedAudio(const DumpRecordedAudio &rhs) :
        ActionBase(rhs),
        targetBuffer(rhs.targetBuffer),
        targetFile(rhs.targetFile),
        size(rhs.size),
        fileStartPosition(rhs.fileStartPosition),
        trackId(rhs.trackId) {}

    std::type_index actionType() const override { return typeid(DumpRecordedAudio); }

    AudioBuffer * targetBuffer;
    AudioFile * targetFile;
    frame_t size;
    frame_t fileStartPosition;
    ID trackId;
};

struct DumpRecordedMidi {};

struct RecordArm : public ActionBase {
    RecordArm() {}
    RecordArm(const RecordArm &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        recordState(rhs.recordState),
        recordSource(rhs.recordSource) {}

    std::type_index actionType() const override { return typeid(RecordArm); }

    ID targetId;
    float recordState;
    RecordSource recordSource;
};

struct ReinitTrackRecord : public ActionBase {
    ReinitTrackRecord() {}
    ReinitTrackRecord(const ReinitTrackRecord &rhs) :
        ActionBase(rhs),
        track(rhs.track) {}

    std::type_index actionType() const override { return typeid(ReinitTrackRecord); }

    Track * track;
};

}

namespace RtTasks {

struct DumpAudioFlat : public FlatTask {
    void execRT();

    AudioBuffer * targetBuffer;
    AudioFile * targetFile;
    frame_t size;
    frame_t fileStartPosition;
    ID trackId;
};

struct ReinitTrackFlat : public FlatTask {
    void execRT();

    Track * track;
};

}

}
