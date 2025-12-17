/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/AudioBuffer.h"
#include <string>
#include <vector>
#include "Status.h"
#include "defines.h"
#include <cstdint> 

namespace slr {

class Track;
class ContainerItem;
class File;
class AudioFile;

namespace Events {

struct RecordArm {
    ID targetId;
    float recordState; //on off
    RecordSource recordSource; //audio midi 0.0 - Audio, 1.0 - midi
};
struct OpenFile {
	ID targetId;
    std::string path;
};
struct FileOpened {
    Status status;
    File * file; //-> class File;
	ID targetId;
};
struct RemoveFile {
    ID fileId;
    ID trackId;
};
struct FileUIRemoved {
    ID fileId;
    ID trackId;
};
struct ModContainerItem {
    ID unitId;
    ID itemId;
    frame_t startPosition;
    frame_t length;
    bool muted;
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
struct AppendItem {
	Track * track; //-> class Track;
	ContainerItem * item; //->class ContainerItem;
};
struct SwapContainer {
    Track * track; //-> class Track;
    std::vector<ContainerItem*> * container; //-> class ContainerItem;
};
struct ModContainerItem {
    Track * track; //-> class Track;
    ContainerItem * item; //->class ContainerItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
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
struct AppendItem {
	Track * track; //-> class Track;
	ContainerItem * item; //->class ContainerItem;
};
struct SwapContainer {
    Track * track;
    std::vector<ContainerItem*> * oldContainer; //-> class ContainerItem;
    std::vector<ContainerItem*> * newContainer;
};
struct ModContainerItem {
    Track * track; //-> class Track;
    ContainerItem * item; //->class ContainerItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
