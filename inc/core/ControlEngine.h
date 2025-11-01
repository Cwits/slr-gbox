// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/Events.h"
#include "core/FlatEvents.h"
#include "core/primitives/ControlContext.h"
#include "defines.h"

#include <functional>
namespace slr {

class ProjectView;
class DriverView;
class RtEngine;
class FileWorker;

namespace ControlEngine {

bool init();
bool shutdown();
void emergencyStop();

const ID generateCommandId();
void EmitEvent(const Events::Event &e);
void emitRtControl(const FlatEvents::FlatControl &ctl);
void emitRtResponse(const FlatEvents::FlatResponse &resp);

void awaitRtResult(const FlatEvents::FlatControl &ctl,
                std::function<void(const ControlContext&, 
                    const FlatEvents::FlatResponse&)> clb);

void notify();

void checkMidiDevices();

ProjectView * projectSnapshot();
DriverView * driverSnapshot();
RtEngine * rtEngine();
FileWorker * fileWorker();

} //namespace ControlEngine


}