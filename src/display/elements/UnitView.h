// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"

namespace UI {
class UIContext;
class UnitUIBase;

class UnitView : public BaseWidget {
    public:
    UnitView(BaseWidget * parent, UIContext * const uictx);
    ~UnitView();

    void show() override;
    void pollUIUpdate() override;

    lv_obj_t * _lb;
    private:
    UIContext * const _uictx;
    UnitUIBase * _lastShownModule;
    
    bool handleDrag(const GestLib::DragGesture & drag);
};

}