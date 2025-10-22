/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"
#include "Status.h"
#include "core/primitives/AudioBuffer.h"

namespace slr {

class AudioFile;
class Track;

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
