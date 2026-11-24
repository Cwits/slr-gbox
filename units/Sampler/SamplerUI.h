// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "display/primitives/UnitUIBase.h"
#include "snapshots/ClipContainerView.h"
#include "units/Sampler/SamplerCommon.h"

#include <memory>
#include <array>

namespace slr {
    class SamplerView;
    class AudioUnitView;
}

namespace UI {

class Label;
class Button;
struct DropDown;
class UIContext;
struct Timeline;
struct FilePreview;

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

        std::unique_ptr<Label> _lblRootPitchStatic;
        std::unique_ptr<Label> _lblRootPitch;
        std::unique_ptr<Label> _lblMaxVoicesStatic;
        std::unique_ptr<Label> _lblMaxVoices;
        std::unique_ptr<Label> _lblSampleStartStatic;
        std::unique_ptr<Label> _lblSampleStart;
        std::unique_ptr<Label> _lblSampleEndStatic;
        std::unique_ptr<Label> _lblSampleEnd;

        std::unique_ptr<Label> _lblModeStatic;
        std::unique_ptr<DropDown> _ddMode;

        lv_timer_t * _updateTimer;

        bool _lastAssetState;
        std::unique_ptr<Label> _lblAsset;

        double _horizontalZoom;
        std::unique_ptr<Timeline> _timeline;

        slr::ClipContentView _content;
        std::unique_ptr<FilePreview> _preview;

        struct VoicePlayback {
            bool active;
            lv_obj_t *line;
            lv_point_precise_t points[2];
        };
        
        std::array<VoicePlayback, MAX_VOICES> _voiceLines;
        
        bool handleDrag(const GestLib::DragGesture & drag);
        
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