// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// #include "core/Events.h"
// #include "core/FlatEvents.h"
#include "core/primitives/ControlContext.h"
#include "defines.h"

#include <functional>
#include <memory>

namespace slr {

class ProjectView;
class DriverView;
class RtEngine;
class FileWorker;
class MidiController;
class ActionBase;

namespace ControlEngine {

bool init();
bool shutdown();
void emergencyStop();

const ID generateCommandId();

void EmitAction(std::unique_ptr<ActionBase> action);

// void checkMidiDevices();

ProjectView * projectSnapshot();
DriverView * driverSnapshot();
RtEngine * rtEngine();
FileWorker * fileWorker();
MidiController * midiController();

} //namespace ControlEngine


}