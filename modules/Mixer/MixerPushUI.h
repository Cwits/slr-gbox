// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/push/primitives/UnitUIBase.h"

#include <memory>

namespace slr {
    class MixerView;    
    class AudioUnitView;
}

namespace PushUI {

class PushUIContext;
class MixerPushUI;

struct MixerGridUI : public DefaultGridUI {
    MixerGridUI(PushLib::Widget *parent, MixerPushUI * parentUI);
    ~MixerGridUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    MixerPushUI * _parentUI;
    // uint64_t _customVersion;
    
    friend class MixerPushUI;
};

struct MixerUnitUI : public DefaultUnitUI {
    MixerUnitUI(PushLib::Widget *parent, MixerPushUI * parentUI);
    ~MixerUnitUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;
    
    private:
    MixerPushUI * _parentUI;
    
    friend class MixerPushUI;
};


struct MixerPushUI : public UnitUIBase {
    MixerPushUI(slr::AudioUnitView * mix, PushUIContext * uictx);
    ~MixerPushUI();
    
    bool create(PushUIContext * ctx) override;
    bool destroy(PushUIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridUI.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    slr::MixerView * const _mixer;

    std::unique_ptr<MixerGridUI> _gridUI;
    std::unique_ptr<MixerUnitUI> _unitUI;

};

}