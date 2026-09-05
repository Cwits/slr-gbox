// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "display/primitives/UnitUIBase.h"
#include <memory>

namespace slr {
    class SamplerView;    
    class AudioUnitView;
}

namespace UI {

class Label;
class Button;
class UIContext;

struct SamplerUI : public UnitUIBase {
    SamplerUI(const std::shared_ptr<const slr::AudioUnitView> &sampler, UIContext * uictx);
    ~SamplerUI();
    
    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    const std::weak_ptr<const slr::SamplerView> _sampler;

    class SamplerGridControlUI;
    class SamplerUnitUI;
    // class SamplerPatchUI;

    std::unique_ptr<SamplerGridControlUI> _gridControl;
    std::unique_ptr<SamplerUnitUI> _unitUI;

    struct SamplerGridControlUI : public DefaultGridUI {
        SamplerGridControlUI(BaseWidget *parent, SamplerUI * parentUI);
        ~SamplerGridControlUI();

        void pollUIUpdate() override;

        private:
        SamplerUI * _parentUI;
        uint64_t _customVersion;

        friend class SamplerUI;
    };

    // struct TrackModuleUI : public DefaultModuleUI {
    struct SamplerUnitUI : public DefaultUnitUI {
        SamplerUnitUI(BaseWidget *parent, SamplerUI * parentUI);
        ~SamplerUnitUI();

        void pollUIUpdate() override;
        
        private:
        SamplerUI * _parentUI;

        lv_obj_t * _testRect;
        bool _lastAssetState;
        std::unique_ptr<Label> _lblAsset;

            
        int _canvasWidth;
        int _canvasHeight;

        lv_obj_t * _canvas;
        lv_color_t _peakColor;
        lv_color_t _fillColor;
        uint8_t * _drawBuffer;

        bool handleDrag(GestLib::DragGesture & drag) override;
        
        friend class SamplerUI;
    };

    // struct SamplerPatchUI : public BaseWidget {   
    //     SamplerPathUI(SamplerUI *parent);
    //     ~SamplerPathUI();

    //     private:
    //     SamplerUI * _parentUI;
    
    //     Label * _name;
    // };
};

}