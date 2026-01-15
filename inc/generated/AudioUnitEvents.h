/* This file is generated automatically, do not edit manually */
#pragma once
#include <cstdint> 
#include "core/FileTasks.h"
#include "core/FileWorker.h"
#include <string>
#include "Status.h"
#include "defines.h"
#include <vector>
#include <memory>

namespace slr {

class File;
class AudioUnit;
class ClipItem;

namespace Events {

struct SetParameter {
    ID targetId;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    ID targetId;
    bool newState;
};
struct ToggleOmniHwInput {
    ID targetId;
    bool newState;
};
struct OpenFile {
	ID unitId;
    std::string path;
	frame_t fileStartPosition;  
};
struct FileOpened {
    Status status;
    const File * file; //-> class File;
	ID unitId;
	frame_t fileStartPosition;
};
struct RemoveFile {
    ID fileId;
    ID unitId;
};
struct FileUIRemoved {
    ID fileId;
    ID unitId;
};
struct ModClipItem {
    ID unitId;
    ID itemId;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct ToggleOmniHwInput {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ClipItem * item; //-> class ClipItem;
};
struct ModClipItem { 
    AudioUnit * unit; //-> class AudioUnit;
    ClipItem * item; //-> class ClipItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
};
struct SwapContainer {
    AudioUnit * unit; //-> class AudioUnit;
    std::vector<ClipItem*> * container; //-> class ClipItem;
};

} //namespace FlatControls

namespace FlatResponses {

struct SetParameter {
    AudioUnit * unit; //-> class AudioUnit;
    ID parameterId;
    float value;
};
struct ToggleMidiThru {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct ToggleOmniHwInput {
    AudioUnit * unit; //-> class AudioUnit;
    bool newState;
};
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ClipItem * item; //-> class ClipItem;
};
struct ModClipItem { 
    AudioUnit * unit; //-> class AudioUnit;
    ClipItem * item; //-> class ClipItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
};
struct SwapContainer {
    AudioUnit * unit;
    std::vector<ClipItem*> * oldContainer; //-> class ClipItem;
    std::vector<ClipItem*> * newContainer;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
