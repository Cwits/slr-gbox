// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"
#include <memory>

namespace slr {
    class AudioUnitView;
    class MixerView;
}

namespace UI {

class Label;
class Button;
class FileView;
class UIContext;
class Slider;

struct MixerUI : public UnitUIBase {
    MixerUI(slr::AudioUnitView * mixer, UIContext * uictx);
    ~MixerUI();

    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultModuleUI * moduleUI() override { return _moduleUI.get(); }

    private:
    slr::MixerView * const _mixer;
    std::vector<FileView*> _viewItems;

    class MixerModuleUI;

    std::unique_ptr<DefaultGridUI> _gridControl;
    std::unique_ptr<MixerModuleUI> _moduleUI;

    struct MixerModuleUI : public DefaultModuleUI { 
        MixerModuleUI(BaseWidget *parent, MixerUI *parentUI);
        ~MixerModuleUI();

        void show() override;
        void pollUIUpdate() override;

        private:
        MixerUI * _parentUI;

        struct SliderWork {
            SliderWork();
            ~SliderWork();
            std::unique_ptr<Slider> slider;
            const slr::AudioUnitView * target;
            uint64_t _lastTargetVersion;
            bool checked;

            SliderWork(SliderWork&&) = default;
            SliderWork& operator=(SliderWork&&) = default;
        };

        std::vector<SliderWork> _sliders;
        
        void checkAddOrDeleteSliders();

        friend class MixerUI;
    };
};

}