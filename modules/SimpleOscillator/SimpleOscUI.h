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
    SimpleOscUI(slr::AudioUnitView * osc, UIContext * uictx);
    ~SimpleOscUI();
    
    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultModuleUI * moduleUI() override { return _moduleUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    slr::SimpleOscView * _osc;

    class SimpleOscModuleUI;

    std::unique_ptr<DefaultGridUI> _gridControl;
    std::unique_ptr<SimpleOscModuleUI> _moduleUI;

    struct SimpleOscModuleUI : public DefaultModuleUI {
        SimpleOscModuleUI(BaseWidget *parent, SimpleOscUI * parentUI);
        ~SimpleOscModuleUI();

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