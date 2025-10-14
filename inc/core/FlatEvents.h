/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "generated/ProjectEvents.h"
#include "generated/RenderPlanEvents.h"
#include "generated/MetronomeEvents.h"
#include "generated/TimelineEvents.h"
#include "generated/AudioUnitEvents.h"
#include "generated/TrackEvents.h"

#include "Status.h"
#include "defines.h"
#include <cstdint>

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
		SetParameter,
		AppendItem,
		SwapContainer,
		ModContainerItem,
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
		FlatControls::SetParameter setParameter;
		FlatControls::AppendItem appendItem;
		FlatControls::SwapContainer swapContainer;
		FlatControls::ModContainerItem modContainerItem;
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
		SetParameter,
		AppendItem,
		SwapContainer,
		ModContainerItem,
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
		FlatResponses::SetParameter setParameter;
		FlatResponses::AppendItem appendItem;
		FlatResponses::SwapContainer swapContainer;
		FlatResponses::ModContainerItem modContainerItem;
		FlatResponses::RecordArm recordArm;
		FlatResponses::DumpRecordedAudio dumpRecordedAudio;
		FlatResponses::ReinitTrackRecord reinitTrackRecord;
	};
};

} //namespace FlatEvents

} //namespace slr
