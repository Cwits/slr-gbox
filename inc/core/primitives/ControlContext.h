// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"

namespace slr {

class Project;
class FileWorker;
class ProjectView;
class RtEngine;
class MidiController;
class BufferManager;

struct ControlContext {
    ControlContext(Project *prj, FileWorker *fw, RtEngine *rt, ProjectView *pv, MidiController *mc, BufferManager *bm) :
        project(prj), fileWorker(fw), engine(rt), projectView(pv), midiController(mc), bufferManager(bm) {}
    Project * const project;
    FileWorker * const fileWorker;
    RtEngine * const engine;
    ProjectView * const projectView;
    MidiController * const midiController;
    BufferManager * const bufferManager;
};

}
