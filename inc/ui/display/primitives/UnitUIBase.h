// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "Color.h"
#include "defines.h"

namespace slr {
    class AudioUnitView;
}

namespace UI {

class Button;
class UIContext;

struct GridBase : public BaseWidget {
    GridBase(BaseWidget * parent, bool hasHost, bool addAsChild) 
            : BaseWidget(parent, hasHost, addAsChild) {}
    virtual ~GridBase() {}
};

struct ModuleBase : public BaseWidget {
    ModuleBase(BaseWidget * parent, bool hasHost, bool addAsChild) 
            : BaseWidget(parent, hasHost, addAsChild) {}
    virtual ~ModuleBase() {}
};

struct UnitUIBase {
    UnitUIBase(slr::AudioUnitView * view);
    virtual ~UnitUIBase();

    virtual bool create(UIContext * ctx) = 0;
    virtual bool update(UIContext * ctx) = 0;
    virtual bool destroy(UIContext * ctx) = 0;

    virtual BaseWidget * gridUI() = 0;
    virtual BaseWidget * moduleUI() = 0;
    // virtual BaseWidget * patchUI() = 0;

    void updateParameter(slr::ID parameterID, float value);
    const slr::ID id() const;
    const slr::Color & color() const;
    const bool canLoadFiles() const { return _canLoadFiles; }

    //there should be better solution for this things...
    virtual int gridY() { return 0; }
    virtual void setNudge(slr::frame_t nudge, const float horizontalZoom) {}
    virtual void updatePosition(int x, int y) {}

    const slr::AudioUnitView * view() const { return _view; }

    // virtual GridBase * grid() = 0;
    // virtual ModuleBase * module() = 0;
    

    protected:
    bool _canLoadFiles = false;
    slr::AudioUnitView * _view;
};


struct UnitControlPopup : public Popup {
    UnitControlPopup(BaseWidget *parent, UIContext * const uictx);
    ~UnitControlPopup();

    UnitUIBase * _currentUnit;
    
    Button * _deleteBtn;
    Button * _routeManagerBtn;
};

}