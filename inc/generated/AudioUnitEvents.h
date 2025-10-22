/* This file is generated automatically, do not edit manually */
#pragma once
#include <string>
#include <cstdint> 
#include "defines.h"
#include <vector>
#include "Status.h"

namespace slr {

class AudioUnit;
class ContainerItem;
class File;

namespace Events {

struct SetParameter {
    ID targetId;
    ID parameterId;
    float value;
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
    ID targetId;
};
struct FileUIRemoved {
    ID fileId;
    ID targetId;
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
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ContainerItem * item; //-> class ContainerItem;
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
struct AppendItem {
	AudioUnit * unit; //-> class AudioUnit;
	ContainerItem * item; //-> class ContainerItem;
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
