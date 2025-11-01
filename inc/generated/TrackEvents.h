/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/AudioBuffer.h"
#include "defines.h"
#include "Status.h"

namespace slr {

class Track;
class AudioFile;

namespace Events {

struct RecordArm {
    ID targetId;
    float recordState; //on off
    RecordSource recordSource; //audio midi 0.0 - Audio, 1.0 - midi
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct RecordArm {
    Track * track; //-> class Track;
    float recordState;
    RecordSource recordSource;
};
struct ReinitTrackRecord {
    Status status;
    Track * track; //-> class Track;
};

} //namespace FlatControls

namespace FlatResponses {

struct RecordArm {
    Track * track; //-> class Track;
    float recordState;
    RecordSource recordSource;
};
struct DumpRecordedAudio {
    AudioBuffer * targetBuffer;
    AudioFile * targetFile; //-> class AudioFile;
    frame_t size;
    ID trackId;
};
struct ReinitTrackRecord {
    Status status;
    Track * track; //-> class Track;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
