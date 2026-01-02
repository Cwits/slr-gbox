/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "generated/TrackEvents.h"
#include "generated/TimelineEvents.h"
#include "generated/MidiControllerEvents.h"
#include "generated/MetronomeEvents.h"
#include "generated/ProjectEvents.h"
#include "generated/RenderPlanEvents.h"
#include "generated/AudioUnitEvents.h"

#include <variant>

namespace slr {

namespace Events {

using Event = std::variant<
		RecordArm,
		RequestPlayhead,
		ChangeTimelineState,
		ToggleLoop,
		ChangeSigBpm,
		LoopPosition,
		ToggleMidiDevice,
		ToggleMetronome,
		AddNewRoute,
		AddNewMidiRoute,
		DeleteModule,
		CreateModule,
		SetParameter,
		ToggleMidiThru,
		ToggleOmniHwInput,
		OpenFile,
		FileOpened,
		RemoveFile,
		FileUIRemoved,
		ModClipItem
>;

} //namespace Events

void EmitEvent(const Events::Event & e);
class ProjectView;
ProjectView * getLastSnapshot();

} //namespace slr