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
    MixerUI(const std::shared_ptr<const slr::AudioUnitView> &mixer, UIContext * uictx);
    ~MixerUI();

    bool create(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridControl.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }

    private:
    const std::weak_ptr<const slr::MixerView> _mixer;
    std::vector<FileView*> _viewItems;

    class MixerUnitUI;

    std::unique_ptr<DefaultGridUI> _gridControl;
    std::unique_ptr<MixerUnitUI> _unitUI;

    struct MixerUnitUI : public DefaultUnitUI { 
        MixerUnitUI(BaseWidget *parent, MixerUI *parentUI);
        ~MixerUnitUI();

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