// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"

#include <functional>
#include <memory>

namespace slr {

struct ProjectView;
struct DriverView;
struct RtEngine;
struct FileWorker;
struct MidiController;
struct ActionBase;
struct BufferManager;

namespace ControlEngine {

bool init();
bool shutdown();
void emergencyStop();

void prepareForProjectLoading();

void EmitAction(std::unique_ptr<ActionBase> action);

// void checkMidiDevices();

ProjectView * projectSnapshot();
DriverView * driverSnapshot();
RtEngine * rtEngine();
FileWorker * fileWorker();
MidiController * midiController();
BufferManager * bufferManager();

} //namespace ControlEngine


}