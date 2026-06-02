// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include <deque>
#include <memory>

namespace slr {

struct Undoable;
class Project;
class FileWorker;
class ProjectView;
class RtEngine;
class MidiController;
class BufferManager;

struct RtTask;

struct ControlContext {
    ControlContext(
        Project *prj, 
        FileWorker *fw, 
        RtEngine *rt, 
        ProjectView *pv, 
        MidiController *mc, 
        BufferManager *bm,
        std::deque<std::unique_ptr<Undoable>> *undo,
        std::deque<std::unique_ptr<Undoable>> *redo
    ) : project(prj), 
        fileWorker(fw), 
        engine(rt), 
        projectView(pv), 
        midiController(mc), 
        bufferManager(bm), 
        _undo(undo), 
        _redo(redo),
        _nonConstEngine(rt) {} 

    Project * const project;
    FileWorker * const fileWorker;
    RtEngine * const engine;
    ProjectView * const projectView;
    MidiController * const midiController;
    BufferManager * const bufferManager;
    
    std::deque<std::unique_ptr<Undoable>> * const _undo;
    std::deque<std::unique_ptr<Undoable>> * const _redo;

    //placeholder for future improvements
    bool prohibitAllocation(std::size_t size) const { 
        if(size == 0) {
            //probably unknown size...
            return false;
        }
        
        std::size_t remainedSpace = 9999;
        if(remainedSpace < size) {
            //return true;
        }
        
        return false; 
    }

    bool prohibitAllocation() const { 
        return false;
    }
    
    ID nextAudioUnitId() const;

    void EmitRtTask(RtTask * task);

    private:
    RtEngine * _nonConstEngine;
};

}
