/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "generated/ProjectEvents.h"
#include "generated/ProjectHandles.h"
#include "generated/RenderPlanEvents.h"
#include "generated/RenderPlanHandles.h"
#include "generated/MetronomeEvents.h"
#include "generated/MetronomeHandles.h"
#include "generated/TimelineEvents.h"
#include "generated/TimelineHandles.h"
#include "generated/AudioUnitEvents.h"
#include "generated/AudioUnitHandles.h"
#include "generated/TrackEvents.h"
#include "generated/TrackHandles.h"

#include "core/Events.h"
#include "core/FlatEvents.h"
#include "core/primitives/ControlContext.h"

namespace slr {

namespace Handlers {

using EventHandlerFn = void(*)(const ControlContext &ctx, const Events::Event&);
using ResponseHandlerFn = void(*)(const ControlContext &ctx, const FlatEvents::FlatResponse&);

template<typename T>
void dispatchHelper(const ControlContext &ctx, const Events::Event& ev) {
    const auto& e = std::get<T>(ev);
    handleEvent(ctx, e); // calls the specific handler
}

extern "C" {
constexpr EventHandlerFn ControlTable[] = {
    //&dispatchHelper<Events::RecordArm>,
	&dispatchHelper<Events::NewTrack>,
	&dispatchHelper<Events::DeleteTrack>,
	&dispatchHelper<Events::AddNewRoute>,
	&dispatchHelper<Events::ToggleMetronome>,
	&dispatchHelper<Events::RequestPlayhead>,
	&dispatchHelper<Events::ChangeTimelineState>,
	&dispatchHelper<Events::ToggleLoop>,
	&dispatchHelper<Events::ChangeSigBpm>,
	&dispatchHelper<Events::LoopPosition>,
	&dispatchHelper<Events::SetParameter>,
	&dispatchHelper<Events::OpenFile>,
	&dispatchHelper<Events::FileOpened>,
	&dispatchHelper<Events::RemoveFile>,
	&dispatchHelper<Events::FileUIRemoved>,
	&dispatchHelper<Events::ModContainerItem>,
	&dispatchHelper<Events::RecordArm>
};
}

extern "C" {
constexpr ResponseHandlerFn ResponseTable[] = {
    //&handleAppendItemResponse,
	&handleGraphSwapped,
	&handleRequestPlayhead,
	&handleRequestChangeTimelineState,
	&handleToggleLoop,
	&handleChangeSigBpm,
	&handleLoopPositionChange,
	&handleSetParameterResponse,
	&handleAppendItemResponse,
	&handleContainerSwapped,
	&handleModifyContainerItemResponse,
	&handleRecordArmResponse,
	&handleDumpRecordedAudio,
	&handleReinitTrackRecord
};
}

} //Handlers

} //namespace slr