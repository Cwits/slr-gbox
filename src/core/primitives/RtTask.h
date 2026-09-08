// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ClipContainer.h"
#include "common/defines.h"
#include <atomic>

namespace slr {

struct FlatTask {

};

struct RtTask {
    void (*fn)(FlatTask*);
    FlatTask *obj;
};

template<typename T>
void wrapper(FlatTask *base) {
    return static_cast<T*>(base)->execRT();
}

template<typename T>
RtTask makeRtTask(T* obj) {
    return RtTask{&wrapper<T>, obj};
}


struct Project;
struct AudioUnit;
struct RenderPlan;
struct RtEngine;

namespace RtTasks {

//tasks that may be used in various actions
struct SetParameterFlat : public FlatTask {
    void execRT();

    AudioUnit * target;
    ID parameterId;
    float value;
    std::atomic<bool> completed;
};

struct SwapRenderPlan : public FlatTask {
	void execRT();
	
	const RenderPlan * plan;
    RtEngine * engine;
	std::atomic<bool> completed;
};

struct SwapContainerFlat : public FlatTask {
    void execRT();

    AudioUnit * target;
    const ClipContainer * container;
    std::atomic<bool> completed;
};


}

}