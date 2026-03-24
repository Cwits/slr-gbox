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
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    BaseWidget * moduleUI() override { return _moduleUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    slr::TrackView * const _track;

    class TrackGridControlUI;
    class TrackModuleUI;
    // class TrackPatchUI;

    std::unique_ptr<TrackGridControlUI> _gridControl;
    std::unique_ptr<TrackModuleUI> _moduleUI;

    struct TrackGridControlUI : public DefaultGridUI {
        TrackGridControlUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackGridControlUI();

        void pollUIUpdate() override;

        private:
        TrackUI * _parentUI;
        uint64_t _customVersion;

        std::unique_ptr<Button> _btnRecord;
        std::unique_ptr<Button> _btnSource;

        friend class TrackUI;
    };

    // struct TrackModuleUI : public DefaultModuleUI {
    struct TrackModuleUI : public BaseWidget {
        TrackModuleUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackModuleUI();

        void pollUIUpdate() override;
        
        private:
        TrackUI * _parentUI;
    
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