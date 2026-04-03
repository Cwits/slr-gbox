/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "generated/ProjectEvents.h"
#include "generated/MetronomeEvents.h"
#include "generated/TimelineEvents.h"
#include "generated/MidiControllerEvents.h"
#include "generated/RenderPlanEvents.h"
#include "generated/AudioUnitEvents.h"
#include "generated/SamplerEvents.h"
#include "generated/TrackEvents.h"

#include <variant>

namespace slr {

namespace Events {

using Event = std::variant<
		AddNewRoute,
		AddNewMidiRoute,
		DeleteModule,
		CreateModule,
		ModClipItem,
		ToggleMetronome,
		RequestPlayhead,
		ChangeTimelineState,
		ToggleLoop,
		ChangeSigBpm,
		LoopPosition,
		ToggleMidiDevice,
		VirtualMidiKbdAction,
		SetParameter,
		ToggleMidiThru,
		ToggleOmniHwInput,
		OpenFile,
		FileOpened,
		RemoveClip,
		ClipUIRemoved,
		LoadAsset,
		AssetOpened,
		RecordArm
>;

} //namespace Events

void EmitEvent(const Events::Event & e);
// bool EmitEventBlocking(const Events::Event &e, int msTimeout); //return false on event failed or timeout(ms)
class ProjectView;
ProjectView * getLastSnapshot();

} //namespace slr