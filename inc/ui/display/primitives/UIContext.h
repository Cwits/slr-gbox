// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/DragContext.h"
#include "ui/display/PopupManager.h"
#include "gestlib/GestLib.h"
#include <vector>

namespace UI {

class UnitUIBase;
class Timeline;
class MainWindow;

class TopPanel;
class BottomPanel;
class GridView;
class GridControl;
class GridGrid;
class ModuleView;
class Browser;

enum class MainView {
    Grid,
    Module, 
    Browser,
    Patch,
    Editor, //audio or midi or automation
    StepSequencer,
    ModMatrix,
};

struct UIContext {
    DragContext _dragContext;
    std::vector<UnitUIBase*> _unitsUI;
    
    PopupManager * _popManager;
    Timeline * _gridTimeline;
    
    BaseWidget * topPanel();
    BaseWidget * bottomPanel();
    BaseWidget * gridControl();
    BaseWidget * grid();
    BaseWidget * moduleView();
    BaseWidget * browser();

    void switchToView(MainView view);
    //void gotoPrevTab();
    MainView previousView();

    void transferGesture(MainView view, GestLib::Gestures gesture);
    void clearHitTestTarget();
    bool cancleGesture(BaseWidget * widget);

    void floatingText(bool warn, std::string text);

    void setLastSelected(UnitUIBase * mod);
    UnitUIBase * getLastSelected();

    float gridHorizontalZoom();

    private:
    TopPanel * _topPanel;
    BottomPanel * _bottomPanel;
    GridView * _gridView;
    ModuleView * _module;
    Browser * _browser;
    //BaseWidget * _patch;

    UnitUIBase * _lastSelectedModule = nullptr;

    MainWindow * _mainWindow;
    friend class MainWindow;
};

/*
as soon as drag initiated with some object(except in gridview i guess)
open view selector popup that will allow to transfer gesture to whatevet other view
e.g. grabbed audio/midi file from track module view -> can transfer to other track, or to editor, 
or somewhere else
grabbed file path from file browser -> can transfer to whatevent, because it can be audio, 
midi, preset, or something else...
and etc...
*/
}