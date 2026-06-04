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
    TrackUI(const std::shared_ptr<const slr::AudioUnitView> &track, UIContext * uictx);
    ~TrackUI();
    
    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    const std::weak_ptr<const slr::TrackView> _track;

    class TrackGridControlUI;
    class TrackUnitUI;
    // class TrackPatchUI;

    std::unique_ptr<TrackGridControlUI> _gridControl;
    std::unique_ptr<TrackUnitUI> _unitUI;

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
    struct TrackUnitUI : public DefaultUnitUI {
        TrackUnitUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackUnitUI();

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