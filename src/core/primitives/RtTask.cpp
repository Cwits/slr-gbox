// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/primitives/RtTask.h"

#include "core/Project.h"
#include "core/RtEngine.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioFile.h"
// #include "core/primitives/AudioFile.h"

namespace slr {

namespace RtTasks {


void SetParameterFlat::execRT() {
    target->setParameter(parameterId, value);
    completed.store(true, std::memory_order_release);
}

void SwapRenderPlan::execRT() {
    engine->SwapRenderPlan(plan);
	completed.store(true, std::memory_order_release);
}

void SwapContainerFlat::execRT() {
    target->setClipContainer(container);
    completed.store(true, std::memory_order_release);
}

void SwapAudioFileData::execRT() {
    for(auto &clip : clips) {
        clip->_content.audio = buf.get();
        if(clip->length() > buf->size()) {
            clip->update(clip->startPosition(), buf->size(), clip->fileOffset(), clip->isMuted());
        }
    }
    file->swapData(buf);
    completed.store(true, std::memory_order_release);
}

}


}