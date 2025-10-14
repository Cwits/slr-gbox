// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"

namespace slr {

class Project;
class FileWorker;
class ProjectView;
class RtEngine;

struct ControlContext {
    Project * project;
    FileWorker * fileWorker;
    RtEngine * engine;
    ProjectView * projectView;
};

}