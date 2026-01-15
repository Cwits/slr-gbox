/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "generated/ProjectEvents.h"
#include "generated/RenderPlanEvents.h"
#include "generated/MetronomeEvents.h"
#include "generated/TimelineEvents.h"
#include "generated/MidiControllerEvents.h"
#include "generated/AudioUnitEvents.h"
#include "generated/TrackEvents.h"

#include "Status.h"
#include "defines.h"
#include <cstdint>
#include <assert.h>

namespace slr {

namespace FlatEvents {

struct FlatControl {
	enum class Type { //Error = 0, 
		SwapRenderPlan,
		RequestPlayhead,
		ChangeTimelineState,
		ToggleLoop,
		ChangeSigBpm,
		LoopPosition,
		UpdateMidiMaps,
		SetParameter,
		ToggleMidiThru,
		ToggleOmniHwInput,
		AppendItem,
		ModClipItem,
		SwapContainer,
		RecordArm,
		ReinitTrackRecord
	};
	Type type;
	ID commandId;
	union {
		FlatControls::SwapRenderPlan swapRenderPlan;
		FlatControls::RequestPlayhead requestPlayhead;
		FlatControls::ChangeTimelineState changeTimelineState;
		FlatControls::ToggleLoop toggleLoop;
		FlatControls::ChangeSigBpm changeSigBpm;
		FlatControls::LoopPosition loopPosition;
		FlatControls::UpdateMidiMaps updateMidiMaps;
		FlatControls::SetParameter setParameter;
		FlatControls::ToggleMidiThru toggleMidiThru;
		FlatControls::ToggleOmniHwInput toggleOmniHwInput;
		FlatControls::AppendItem appendItem;
		FlatControls::ModClipItem modClipItem;
		FlatControls::SwapContainer swapContainer;
		FlatControls::RecordArm recordArm;
		FlatControls::ReinitTrackRecord reinitTrackRecord;
	};
};

struct FlatResponse {
	enum class Type { //Error = 0, 
		SwapRenderPlan,
		RequestPlayhead,
		ChangeTimelineState,
		ToggleLoop,
		ChangeSigBpm,
		LoopPosition,
		UpdateMidiMaps,
		SetParameter,
		ToggleMidiThru,
		ToggleOmniHwInput,
		AppendItem,
		ModClipItem,
		SwapContainer,
		RecordArm,
		DumpRecordedAudio,
		ReinitTrackRecord
	}; 
	Type type;
	Status status;
	ID commandId;
	union {
		FlatResponses::SwapRenderPlan swapRenderPlan;
		FlatResponses::RequestPlayhead requestPlayhead;
		FlatResponses::ChangeTimelineState changeTimelineState;
		FlatResponses::ToggleLoop toggleLoop;
		FlatResponses::ChangeSigBpm changeSigBpm;
		FlatResponses::LoopPosition loopPosition;
		FlatResponses::UpdateMidiMaps updateMidiMaps;
		FlatResponses::SetParameter setParameter;
		FlatResponses::ToggleMidiThru toggleMidiThru;
		FlatResponses::ToggleOmniHwInput toggleOmniHwInput;
		FlatResponses::AppendItem appendItem;
		FlatResponses::ModClipItem modClipItem;
		FlatResponses::SwapContainer swapContainer;
		FlatResponses::RecordArm recordArm;
		FlatResponses::DumpRecordedAudio dumpRecordedAudio;
		FlatResponses::ReinitTrackRecord reinitTrackRecord;
	};
};

// static_assert(sizeof(FlatControl <= 64)); //TODO: ? it is bigger lol :/
static_assert(sizeof(FlatResponse) <= 64);

} //namespace FlatEvents

} //namespace slr
