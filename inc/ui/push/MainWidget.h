// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <memory>
#include <map>

namespace PushLib {
    class Painter;
}

namespace PushUI {

class PadNoteLayout;
class PadLayoutSelector;
// class PadLayout;
// class GridView;
// class ModuleView;
// class Browser;
// class Settings;

enum class PushView {
    Grid, //
    AudioUnit, //track/mixer/osc/whatever
    Browser,    //file browser
    Editor, //midi, audio or automation editor
    Patch,
    ScaleSelector,
    PadLayoutSelector,
};

struct MainWidget : public PushLib::Widget {
    MainWidget(PushLib::PushContext * const pctx);
    ~MainWidget();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    void switchToView(PushView view);

    private:
    std::unique_ptr<PadLayoutSelector> _padLayoutSelector;

    PushView _currentView;
    bool _layoutSwitched;

    int RectX;
    int RectY;
    int RectColor;
    int playColor;
    
    static const PushLib::ButtonCallbackMap<MainWidget> _mainButtonsCallback;

    bool testClb(PushLib::ButtonEvent &ev);
    bool scaleSelector(PushLib::ButtonEvent &ev);
    bool layoutSelector(PushLib::ButtonEvent &ev);
    bool playButtonClb(PushLib::ButtonEvent &ev);
};

}