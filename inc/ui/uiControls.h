// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include <memory>
#include "defines.h"

namespace slr {
    class AudioUnitView;
    class TrackView;
    class ContainerItemView;
    class UnitDescriptor;
}

namespace UI {
    class UnitUIBase;
}

namespace UIControls {

void floatingInfo(std::string text);
void floatingWarning(std::string text);

/* Module related */
void addUnitUI(const slr::UnitDescriptor * desc, const std::shared_ptr<const slr::AudioUnitView> view);
void destroyUnitUI(slr::ID id);

/* Timeline */
void updateTimeline(const bool timeSigOrBpm);
void updatePlayheadPosition(slr::frame_t position);

/* Route Manager */
void updateRouteManager();

/* Metronome */
void updateMetronomeState(bool onoff);

void clearUI();

}