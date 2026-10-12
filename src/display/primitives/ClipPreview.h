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
struct FilePreview;

struct ClipPreview : public BaseWidget { //this should be called ClipUI or smth...
    ClipPreview(BaseWidget * parent, UnitUIBase * _parentUI, const slr::ClipItemView * const item, UIContext * const uictx);
    ~ClipPreview();

    void update();
    UnitUIBase * parentUI() const { return _parentUI; }
    slr::ID id() const { return _uniqueId; }
    void recalculateWidthAndRedraw();

    void setSize(lv_coord_t w, lv_coord_t h) override;

    const slr::ClipItemView * const _clipItem;

    void pollUIUpdate() override;
    void draw();

    private:
    UIContext * const _uictx;
    UnitUIBase * _parentUI;
    
    std::unique_ptr<FilePreview> _preview;
    
    const slr::ID _uniqueId;

    int _originalX;
    int _originalY;

    uint64_t _uiVersion;

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
    void setItem(ClipPreview * item);
    const ClipPreview * item() const { return _item; }
 
    private:
    ClipPreview * _item;

    std::unique_ptr<Button> _btnDelete;
    std::unique_ptr<Button> _btnEdit;

    
    void forcedClose() override;
};

}