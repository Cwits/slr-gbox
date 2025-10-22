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

#include <variant>

namespace slr {

namespace Events {

using Event = std::variant<
		AddNewRoute,
		DeleteModule,
		CreateModule,
		ToggleMetronome,
		RequestPlayhead,
		ChangeTimelineState,
		ToggleLoop,
		ChangeSigBpm,
		LoopPosition,
		SetParameter,
		OpenFile,
		FileOpened,
		RemoveFile,
		FileUIRemoved,
		ModContainerItem,
		RecordArm
>;

} //namespace Events

void EmitEvent(const Events::Event & e);
class ProjectView;
ProjectView * getLastSnapshot();

} //namespace slr