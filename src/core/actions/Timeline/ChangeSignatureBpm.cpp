// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/Timeline/ChangeSignatureBpm.h"
#include "core/actions/ActionBase.h"

#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/MidiFile.h"
#include "core/utility/ControlContext.h"
#include "core/Project.h"
#include "core/Timeline.h"
#include "core/FileWorker.h"

#include "snapshots/ProjectView.h"
#include "snapshots/TimelineView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>
#include <memory>
  
namespace slr {
	
ChangeSignatureBpmAction::ChangeSignatureBpmAction(const ActionBase *base) :
	_action( *(static_cast<const Actions::ChangeSignatureBpm*>(base)) ) {
}
ChangeSignatureBpmAction::~ChangeSignatureBpmAction() {}

void ChangeSignatureBpmAction::exec(ControlContext &ctx) {
	assert(getState() == ActionState::Executing);
	
    switch(_step) {
        case(1): {

            _flat.tl = &ctx.project->timeline();

            if(_direction == ActionDirection::Forward) {
                _oldValues.bpm = ctx.project->timeline().bpm();
                _oldValues.sig = ctx.project->timeline().getBarSize();

                _flat.bpm = _action.bpm;
                _flat.sig = _action.sig;
            } else {
                _flat.bpm = _oldValues.bpm;
                _flat.sig = _oldValues.sig;
            }

            std::vector<File*> tmp = ctx.fileWorker->listFiles(FileType::Audio);
            std::vector<AudioFile*> tmp2;
            tmp2.reserve(tmp.size());
            for(auto *f : tmp) tmp2.push_back(static_cast<AudioFile*>(f));
            
            _flat.audiofiles = tmp2;

            tmp.clear();
            tmp = ctx.fileWorker->listFiles(FileType::Midi);
            std::vector<MidiFile*> tmp3;
            tmp3.reserve(tmp.size());
            for(auto *f : tmp) tmp3.push_back(static_cast<MidiFile*>(f));

            _flat.midifiles = tmp3;

            _flat.clips = ctx.project->clipStorage().items();
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
        } break;
        case(2): {
            if(_direction == ActionDirection::Forward) {
                ctx.projectView->timeline().update();
            } else {
                ctx.projectView->timeline().update();
            }
            UIControls::updateTimeline(true);

                
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}
void ChangeSignatureBpmAction::checkWaitingCondition(ControlContext &ctx) {
	assert(getState() == ActionState::Waiting);
	
	switch(_step) {
		case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
		} break;
        default: assert(false && "Unreachable"); break;
	}
}

void ChangeSignatureBpmAction::ChangeSigBpm::execRT() {
    double ofpqn = tl->framesPerQuater();
    tl->setBpm(bpm);
    tl->setBarSize(sig);
    double nfpqn = tl->framesPerQuater();
    double coef = nfpqn / ofpqn;

    //need to recalculate all clip item positions?
    /// yes...
    // double coef = oldBpm / bpm;

    // for(auto *f : audiofiles) f->recalculate(ofpqn, nfpqn);
    // for(auto *f : midifiles) f->recalculate(ofpqn, nfpqn);

    for(auto *c : clips) 
        c->recalculate(coef);

    //as well need to recalculate all midi items, for audio items it would be only playback rate change
    
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createChangeSignatureBpmAction(const ActionBase*base) {
    return std::make_unique<ChangeSignatureBpmAction>(base);
}


}