// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ClipContainer.h"
#include "defines.h"
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

namespace RtTasks {

struct SetParameterFlat : public FlatTask {
    void execRT();

    AudioUnit * target;
    ID parameterId;
    float value;
    std::atomic<bool> completed;
};

struct SwapRenderPlan : public FlatTask {
	void execRT();
	
	Project * project;
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