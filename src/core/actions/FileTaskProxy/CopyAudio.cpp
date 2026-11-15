// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/FileTaskProxy/CopyAudio.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/filetasks/FileTasks.h"
#include "snapshots/ProjectView.h"
#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

CopyAudioAction::CopyAudioAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CopyAudio*>(base)) )
{

}

CopyAudioAction::~CopyAudioAction() {

}

void CopyAudioAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);
    auto fact = std::make_unique<Tasks::CopyAudio>();
    fact->start = _action.start;
    fact->end = _action.end;
    fact->channel = _action.channel;
    fact->fileId = _action.fileId;
    ctx.project->fileWorker()->addTask(std::move(fact));
    setState(ActionState::Finished);
}

void CopyAudioAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

}

std::unique_ptr<ActionExecutable> createCopyAudioAction(const ActionBase *base) {
    return std::make_unique<CopyAudioAction>(base);
}

}