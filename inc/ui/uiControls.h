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

/*
enum class DialogWindowFlags : unsigned int {
    DW_OK               = 0x00000001L,
    DW_OKCANCLE         = 0x00000002L,
    // DW_CANCLEIGNOREFIND = 0x00000003L,

};

enum class DialogWindowResponse : unsigned int {
    DWR_OK                  = 0x00000001L,
    DWR_CANCLE              = 0x00000002L,
    // DWR_FIND                = 0x00000003L, //if this in response than data contains std::string with path to file
};

void DialogWindow(std::string text, std::function<void(unsigned int res)> clb, unsigned int flags);
*/

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