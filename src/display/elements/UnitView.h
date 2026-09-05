// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/View.h"

namespace UI {
class UIContext;
class UnitUIBase;

class UnitView : public View {
    public:
    UnitView(BaseWidget * parent, UIContext * const uictx);
    ~UnitView();

    void update() override;
    void pollUIUpdate() override;

    lv_obj_t * _lb;
    private:

    UnitUIBase * _lastShownModule;
    
    bool handleDrag(GestLib::DragGesture & drag) override;
};

}