// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "defines.h"

namespace slr {
    class ContainerItemView;
}

namespace UI {
// class TrackGui;
class UnitUIBase;
class Button;
class UIContext;

struct FileView : public BaseWidget {
    FileView(BaseWidget * parent, UnitUIBase * _parentUI, slr::ContainerItemView * item, UIContext * const uictx);
    ~FileView();

    void update();
    UnitUIBase * parentUI() const { return _parentUI; }
    slr::ID id() const { return _uniqueId; }

    int _canvasWidth;
    int _canvasHeight;

    lv_obj_t * _canvas;
    lv_color_t _peakColor;
    lv_color_t _fillColor;

    slr::ContainerItemView * _item;
    
    private:
    UIContext * const _uictx;
    UnitUIBase * _parentUI;
    uint8_t * _drawBuffer;
    const slr::ID _uniqueId;

    int _originalX;
    int _originalY;

    void draw();

    bool handleTap(GestLib::TapGesture & tap) override;
    //use Hold for dragging item across grid
    bool handleDoubleTap(GestLib::DoubleTapGesture & dtap) override;
    // bool handleDrag(GestLib::DragGesture & drag);
    bool handleHold(GestLib::HoldGesture &hold) override;
};

struct FilePopup : public Popup {
    FilePopup(BaseWidget * parent, UIContext * const uictx);
    ~FilePopup();

    void update();
    void setItem(FileView * item);
    const FileView * item() const { return _item; }
 
    private:
    FileView * _item;

    Button * _deleteBtn;
    
    void forcedClose() override;
};

}