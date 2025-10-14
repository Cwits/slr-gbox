/* This file is generated automatically, do not edit manually */
#pragma once
#include "defines.h"

namespace slr {

class Timeline;

namespace Events {

struct RequestPlayhead {
};
struct ChangeTimelineState {
    TimelineState state;
};
struct ToggleLoop {
    bool newState;
};
struct ChangeSigBpm {
    float bpm;
    slr::BarSize sig;
};
struct LoopPosition {
    frame_t start;
    frame_t end;
};

} //namespace Events

namespace FlatEvents {

namespace FlatControls {

struct RequestPlayhead {
    Timeline * timeline; //-> class Timeline;
};
struct ChangeTimelineState {
    TimelineState state;
    Timeline * timeline; //-> class Timeline;
};
struct ToggleLoop {
    Timeline * timeline; //-> class Timeline;
    bool newState;
};
struct ChangeSigBpm {
    Timeline * tl; //-> class Timeline;
    float bpm;
    slr::BarSize sig;
};
struct LoopPosition {
    Timeline * tl; //-> class Timeline;
    frame_t start;
    frame_t end;
};

} //namespace FlatControls

namespace FlatResponses {

struct RequestPlayhead {
    frame_t position;
};
struct ChangeTimelineState {
    TimelineState state;
};
struct ToggleLoop {
    Timeline * timeline; //-> class Timeline;
    bool newState;
};
struct ChangeSigBpm {
    Timeline * tl; //-> class Timeline;
    float bpm;
    slr::BarSize sig;
};
struct LoopPosition {
    Timeline * tl; //-> class Timeline;
    frame_t start;
    frame_t end;
};

} //namespace FlatResponses

} //namespace FlatEvents

} //namespace slr
