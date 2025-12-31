// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"

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
    bool update(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    BaseWidget * gridUI() override { return _gridControl; }
    BaseWidget * moduleUI() override { return _moduleUI; }
    // BaseWidget * patchUI() override;
 
    // int gridY() override;
    // void updatePosition(int x, int y) override;

    private:
    slr::SimpleOscView * _osc;

    class SimpleOscGridControlUI;
    class SimpleOscModuleUI;
    // class TrackPatchUI;

    SimpleOscGridControlUI * _gridControl;
    SimpleOscModuleUI * _moduleUI;

    struct SimpleOscGridControlUI : public BaseWidget {
        SimpleOscGridControlUI(BaseWidget *parent, SimpleOscUI * parentUI);
        ~SimpleOscGridControlUI();

        private:
        SimpleOscUI * _parentUI;

        Label * _lblName; //static
        Label * _lblVolume;
        Button * _btnMute;
        Button * _btnSolo;
        Button * _btnRecord;
        Button * _btnSource;

        bool handleDoubleTap(GestLib::DoubleTapGesture &dt);

        friend class SimpleOscUI;
    };

    struct SimpleOscModuleUI : public BaseWidget {
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