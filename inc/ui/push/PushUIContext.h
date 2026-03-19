// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"

#include <memory>

namespace PushLib {
    class PushContext;
}

namespace PushUI {

enum class PushView {
    ERROR = 0,
    Grid, //
    Module, //track/mixer/osc/whatever
    Editor, //midi, audio or automation editor
    Patch, //for Module sends and receives? or it can be in Module itself... 
    StepSequencer,
    ModMatrix,

    Browser,    //file browser
    Metronome,
    
    ScaleSelector,

    PushInternalSettings,
    SLRSettings,
};

class RootWidget;
class GridWidget;
class ModuleUIBase;

struct PushUIContext {
    PushUIContext();

    PushLib::PushContext * const pctx() { return _pctx; }

    bool tryHandleButtonDefault(PushLib::ButtonEvent &ev);
    bool tryHandleEncoderDefault(PushLib::EncoderEvent &ev);

    void switchToView(PushView view);
    void goToPreviousView();

    const PushView currentView() const; 
    const PushView previousView() const;

    private:
    PushLib::PushContext * _pctx;

    RootWidget *_rootWidget;
    GridWidget * _gridWidget;

    // std::vector<std::unique_ptr<ModuleUIBase>> _modulesUI;

    friend class RootWidget;
};

}