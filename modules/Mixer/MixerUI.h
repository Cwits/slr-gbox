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

    BaseWidget * gridUI() override { return _gridControl; }
    BaseWidget * moduleUI() override { return _moduleUI; }

    private:
    slr::MixerView * const _mixer;
    std::vector<FileView*> _viewItems;

    class MixerGridControlUI;
    class MixerModuleUI;

    MixerGridControlUI * _gridControl;
    MixerModuleUI * _moduleUI;

    struct MixerGridControlUI : public BaseWidget {
        MixerGridControlUI(BaseWidget *parent, MixerUI *parentUI);
        ~MixerGridControlUI();

        void pollUIUpdate() override;
        
        private:
        MixerUI * _parentUI;

        Label * _lblName;
        Label * _lblVolume;
        Button * _btnMute;
        Button * _btnSolo;

        bool handleDoubleTap(GestLib::DoubleTapGesture &dt);

        friend class MixerUI;
    };

    struct MixerModuleUI : public BaseWidget { 
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