// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "Color.h"
#include "defines.h"

#include <vector>
#include <functional>

namespace slr {
    class AudioUnitView;
}

namespace UI {

class Button;
class UIContext;
class FileView;

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
    UnitUIBase(slr::AudioUnitView * view, UIContext * uictx);
    virtual ~UnitUIBase();

    virtual bool create(UIContext * ctx) = 0;
    virtual bool update(UIContext * ctx);
    virtual bool destroy(UIContext * ctx);

    virtual BaseWidget * gridUI() = 0;
    virtual BaseWidget * moduleUI() = 0;
    // virtual BaseWidget * patchUI() = 0;

    void updateParameter(slr::ID parameterID, float value);
    const slr::ID id() const;
    const slr::Color & color() const;
    const bool canLoadFiles() const { return _canLoadFiles; }

    //there should be better solution for this things...
    virtual int gridY();
    virtual void setNudge(slr::frame_t nudge, const float horizontalZoom);
    virtual void updatePosition(int x, int y);

    const slr::AudioUnitView * view() const { return _view; }

    // virtual GridBase * grid() = 0;
    // virtual ModuleBase * module() = 0;
    
    void registerExternalUpdate(std::function<void()> clb) { _externalUpdates.push_back(std::move(clb)); }

    std::vector<FileView*> & fileList() { return _viewItems; }

    protected:

    UIContext * const _uictx;
    bool _canLoadFiles = false;
    slr::AudioUnitView * _view;
    
    std::vector<FileView*> _viewItems;

    std::vector<std::function<void()>> _externalUpdates;
};


struct UnitControlPopup : public Popup {
    UnitControlPopup(BaseWidget *parent, UIContext * const uictx);
    ~UnitControlPopup();

    UnitUIBase * _currentUnit;
    
    Button * _deleteBtn;
    Button * _routeManagerBtn;
};

}