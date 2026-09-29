// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "display/primitives/Popup.h"
#include "common/defines.h"

namespace slr {
    class ContainerItemView;
    class ClipItemView;
}

namespace UI {
// class TrackGui;
class UnitUIBase;
class Button;
class UIContext;

struct FileView : public BaseWidget { //this should be called ClipUI or smth...
    // FileView(BaseWidget *parent, const slr::ClipItemView *const item, UIContext *const uictx, int x, int y, int expectedWidth, int expectedHeight);
    FileView(BaseWidget * parent, UnitUIBase * _parentUI, const slr::ClipItemView * const item, UIContext * const uictx);
    ~FileView();

    void update();
    UnitUIBase * parentUI() const { return _parentUI; }
    slr::ID id() const { return _uniqueId; }
    void recalculateWidthAndRedraw();

    int _canvasWidth;
    int _canvasHeight;

    lv_obj_t * _canvas;
    lv_color_t _peakColor;
    lv_color_t _fillColor;

    const slr::ClipItemView * const _clipItem;

    void pollUIUpdate() override;
    
    private:
    UIContext * const _uictx;
    UnitUIBase * _parentUI;
    
    std::unique_ptr<uint8_t[]> _drawBuffer;
    
    const slr::ID _uniqueId;

    int _originalX;
    int _originalY;

    uint64_t _uiVersion;

    void draw();

    bool handleTap(const GestLib::TapGesture & tap);
    //use Hold for dragging item across grid
    bool handleDoubleTap(const GestLib::DoubleTapGesture & dtap);
    // bool handleDrag(GestLib::DragGesture & drag);
    bool handleHold(const GestLib::HoldGesture &hold);
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