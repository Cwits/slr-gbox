// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"
#include <memory>

namespace slr {
    class SimpleOscView;    
    class AudioUnitView;
}

namespace UI {

class Label;
class Button;
class FileView;
class UIContext;
class FileView;

struct SimpleOscUI : public UnitUIBase {
    SimpleOscUI(const std::shared_ptr<const slr::AudioUnitView> &osc, UIContext * uictx);
    ~SimpleOscUI();
    
    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    const std::weak_ptr<const slr::SimpleOscView> _osc;

    class SimpleOscUnitUI;

    std::unique_ptr<DefaultGridUI> _gridControl;
    std::unique_ptr<SimpleOscUnitUI> _unitUI;

    struct SimpleOscUnitUI : public DefaultUnitUI {
        SimpleOscUnitUI(BaseWidget *parent, SimpleOscUI * parentUI);
        ~SimpleOscUnitUI();

        private:
        SimpleOscUI * _parentUI;
    
        Label * _name;
        lv_obj_t * _testRect;
        
        friend class SimpleOscUI;
    };

    // struct TrackPatchUI : public BaseWidget {   
    //     TrackPathUI(TrackUI *parent);
    //     ~TrackPathUI();

    //     private:
    //     TrackUI * _parentUI;
    
    //     Label * _name;
    // };
};

}