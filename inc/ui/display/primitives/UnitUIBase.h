// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "common/Color.h"
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

struct UnitUIBase {
    UnitUIBase(slr::AudioUnitView * view, UIContext * uictx);
    virtual ~UnitUIBase();

    virtual bool create(UIContext * ctx) = 0;
    bool commonUIUpdate();
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

    std::vector<FileView*> & fileList() { return _viewItems; }

    protected:

    UIContext * const _uictx;
    bool _canLoadFiles = false;
    slr::AudioUnitView * _view;
    
    std::vector<FileView*> _viewItems; //only for grid? or both for grid and module?
    uint64_t _fileContainerVersion;
};


struct UnitControlPopup : public Popup {
    UnitControlPopup(BaseWidget *parent, UIContext * const uictx);
    ~UnitControlPopup();

    UnitUIBase * _currentUnit;
    
    Button * _deleteBtn;
    Button * _routeManagerBtn;
};

struct DefaultGridUI : public BaseWidget {
    DefaultGridUI(BaseWidget * parent, UnitUIBase *base) 
            : BaseWidget(parent, true, true) {}
    virtual ~DefaultGridUI() {}
    // std::vector<std::unique_ptr<FileView>> _fileUIs;
};

struct DefaultModuleUI : public BaseWidget {
    DefaultModuleUI(BaseWidget * parent, UnitUIBase *base) 
            : BaseWidget(parent, true, true) {}
    virtual ~DefaultModuleUI() {}
    // std::unique_ptr<FileView> _fileEditorUI;
};

}