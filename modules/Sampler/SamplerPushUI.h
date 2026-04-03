// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ui/push/primitives/UnitUIBase.h"

#include <memory>

namespace slr {
    class SamplerView;    
    class AudioUnitView;
}

namespace PushUI {

class PushUIContext;
class SamplerPushUI;

struct SamplerGridUI : public DefaultGridUI {
    SamplerGridUI(PushLib::Widget *parent, SamplerPushUI * parentUI);
    ~SamplerGridUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    SamplerPushUI * _parentUI;
    // uint64_t _customVersion;
    
    friend class SamplerPushUI;
};

struct SamplerUnitUI : public DefaultUnitUI {
    SamplerUnitUI(PushLib::Widget *parent, SamplerPushUI * parentUI);
    ~SamplerUnitUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;
    
    private:
    SamplerPushUI * _parentUI;
    
    friend class SamplerPushUI;
};


struct SamplerPushUI : public UnitUIBase {
    SamplerPushUI(slr::AudioUnitView * sampler, PushUIContext * uictx);
    ~SamplerPushUI();
    
    bool create(PushUIContext * ctx) override;
    bool destroy(PushUIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridUI.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    slr::SamplerView * const _sampler;

    std::unique_ptr<SamplerGridUI> _gridUI;
    std::unique_ptr<SamplerUnitUI> _unitUI;

};

}