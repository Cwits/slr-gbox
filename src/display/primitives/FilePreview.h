// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "display/primitives/Popup.h"
#include "common/defines.h"

namespace slr {
    class ContainerItemView;
    struct ClipContentView;
}

namespace UI {

class UnitUIBase;
class UIContext;

struct FilePreview : public BaseWidget { 
    FilePreview(BaseWidget * parent, const slr::ClipContentView * const item, UIContext * const uictx);
    ~FilePreview();

    slr::ID id() const;

    void setSize(lv_coord_t w, lv_coord_t h) override;
    void pollUIUpdate() override;
    void draw(slr::frame_t, slr::frame_t, int = -1, int = -1);

    const slr::ClipContentView * const _content;

    lv_color_t _peakColor;
    lv_color_t _fillColor;
    int _bgOpa;
    lv_color_t _clrBackground;

    private:
    UIContext * const _uictx;
    uint64_t _uiVersion;

    int _canvasWidth;
    int _canvasHeight;

    lv_obj_t * _canvas;

    std::unique_ptr<uint8_t[]> _drawBuffer;

    // bool handleTap(const GestLib::TapGesture & tap);
    // //use Hold for dragging item across grid
    // bool handleDoubleTap(const GestLib::DoubleTapGesture & dtap);
    // // bool handleDrag(GestLib::DragGesture & drag);
    // bool handleHold(const GestLib::HoldGesture &hold);
};


}