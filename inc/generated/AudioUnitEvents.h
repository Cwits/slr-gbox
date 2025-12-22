/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"
#include <vector>
#include <string>
#include <cstdint> 
#include "Status.h"

namespace slr {

class File;
class ContainerItem;
class AudioUnit;

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
    ID unitId;
};
struct FileUIRemoved {
    ID fileId;
    ID unitId;
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
	ContainerItem * item; //->class ContainerItem;
};
struct SwapContainer {
    AudioUnit * unit; //-> class AudioUnit;
    std::vector<ContainerItem*> * container; //-> class ContainerItem;
};
struct ModContainerItem {
    AudioUnit * unit; //-> class AudioUnit;
    ContainerItem * item; //->class ContainerItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
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
	ContainerItem * item; //->class ContainerItem;
};
struct SwapContainer {
    AudioUnit * unit;
    std::vector<ContainerItem*> * oldContainer; //-> class ContainerItem;
    std::vector<ContainerItem*> * newContainer;
};
struct ModContainerItem {
    AudioUnit * unit; //-> class AudioUnit;
    ContainerItem * item; //->class ContainerItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
