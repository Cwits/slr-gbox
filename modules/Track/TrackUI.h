// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"

namespace slr {
    class TrackView;    
    class AudioUnitView;
}

namespace UI {

class Label;
class Button;
class UIContext;

struct TrackUI : public UnitUIBase {
    TrackUI(slr::AudioUnitView * track, UIContext * uictx);
    ~TrackUI();
    
    bool create(UIContext * ctx) override;
    bool update(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    BaseWidget * gridUI() override { return _gridControl; }
    BaseWidget * moduleUI() override { return _moduleUI; }
    // BaseWidget * patchUI() override;
 
    private:
    slr::TrackView * _track;

    class TrackGridControlUI;
    class TrackModuleUI;
    // class TrackPatchUI;

    TrackGridControlUI * _gridControl;
    TrackModuleUI * _moduleUI;

    struct TrackGridControlUI : public BaseWidget {
        TrackGridControlUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackGridControlUI();

        private:
        TrackUI * _parentUI;

        Label * _lblName; //static
        Label * _lblVolume;
        Button * _btnMute;
        Button * _btnSolo;
        Button * _btnRecord;
        Button * _btnSource;

        bool handleDoubleTap(GestLib::DoubleTapGesture &dt);

        friend class TrackUI;
    };

    struct TrackModuleUI : public BaseWidget {
        TrackModuleUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackModuleUI();

        private:
        TrackUI * _parentUI;
        // std::vector<FileView*> _viewItems;
    
        Label * _name;
        lv_obj_t * _testRect;
        
        friend class TrackUI;
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