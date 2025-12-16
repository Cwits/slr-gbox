// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"

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
    bool update(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    BaseWidget * gridUI() override { return _gridControl; }
    BaseWidget * moduleUI() override { return _moduleUI; }

    // int gridY() override;
    // void setNudge(slr::frame_t nudge, const float horizontalZoom) override;
    // void updatePosition(int x, int y) override;

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

        private:
        MixerUI * _parentUI;

        // struct sliderUnit {
        //     Label * _lblName;
        //     Slider * _sldVolume;
        // };
        std::vector<Slider*> _sliders; // need somehow to update these thing based on routes

        friend class MixerUI;
    };
};

}