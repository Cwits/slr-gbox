// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"

#include <memory>

namespace PushLib {
    class PushContext;
    class Widget;
}

namespace PushUI {

enum class PushView {
    ERROR = 0,
    Grid, //
    Unit, //track/mixer/osc/whatever
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
class UnitUIBase;
class UnitWidget;

struct PushUIContext {
    PushUIContext();
    ~PushUIContext();

    PushLib::PushContext * const pctx() { return _pctx; }

    bool tryHandleButtonDefault(PushLib::ButtonEvent &ev);
    bool tryHandleEncoderDefault(PushLib::EncoderEvent &ev);

    void switchToView(PushView view);
    void goToPreviousView();

    const PushView currentView() const; 
    const PushView previousView() const;

    PushLib::Widget * gridWidget() const;
    PushLib::Widget * unitWidget() const;

    void forceRedraw();

    std::size_t unitsCount() { return _unitUIs.size(); }

    private:
    PushLib::PushContext * _pctx;

    RootWidget *_rootWidget;
    GridWidget * _gridWidget;
    UnitWidget * _unitWidget;

    std::vector<std::unique_ptr<UnitUIBase>> _unitUIs;

    friend class RootWidget;
};

}