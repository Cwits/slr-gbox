// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include "defines.h"

namespace slr {
    class AudioUnitView;
    class TrackView;
    class ContainerItemView;
    class Module;
}

namespace UI {
    class UnitUIBase;
}

namespace UIControls {

void floatingInfo(std::string text);
void floatingWarning(std::string text);

/* Module related */
void addModuleUI(const slr::Module * mod, slr::AudioUnitView * view);
void updateModuleUI(slr::ID id);
void destroyModuleUI(slr::ID id);

/* Timeline */
void updateTimeline(const bool timeSigOrBpm);
void updatePlayheadPosition(slr::frame_t position);

/* Route Manager */
void updateRouteManager();

/* Metronome */
void updateMetronomeState(bool onoff);

}