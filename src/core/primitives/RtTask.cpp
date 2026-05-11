// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/RtTask.h"

#include "core/Project.h"
#include "core/primitives/AudioUnit.h"

namespace slr {

namespace RtTasks {


void SetParameterFlat::execRT() {
    target->setParameter(parameterId, value);
    completed.store(true, std::memory_order_release);
}

void SwapRenderPlan::execRT() {
	project->swapPlans();
	completed.store(true, std::memory_order_release);
}

void SwapContainerFlat::execRT() {
    target->setClipContainer(container);
    completed.store(true, std::memory_order_release);
}

}


}