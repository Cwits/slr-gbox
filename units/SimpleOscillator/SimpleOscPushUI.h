// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "push/primitives/UnitUIBase.h"

#include <memory>

namespace slr {
    class SimpleOscView;    
    class AudioUnitView;
}

namespace PushUI {

class PushUIContext;
class SimpleOscPushUI;

struct SimpleOscGridUI : public DefaultGridUI {
    SimpleOscGridUI(PushLib::Widget *parent, SimpleOscPushUI * parentUI);
    ~SimpleOscGridUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    SimpleOscPushUI * _parentUI;
    // uint64_t _customVersion;
    
    friend class SimpleOscPushUI;
};

struct SimpleOscUnitUI : public DefaultUnitUI {
    SimpleOscUnitUI(PushLib::Widget *parent, SimpleOscPushUI * parentUI);
    ~SimpleOscUnitUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;
    
    private:
    SimpleOscPushUI * _parentUI;
    
    friend class SimpleOscPushUI;
};


struct SimpleOscPushUI : public UnitUIBase {
    SimpleOscPushUI(const std::shared_ptr<const slr::AudioUnitView> &osc, PushUIContext * uictx);
    ~SimpleOscPushUI();
    
    bool create(PushUIContext * ctx) override;
    bool destroy(PushUIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridUI.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    const std::weak_ptr<const slr::SimpleOscView> _simpleOsc;

    std::unique_ptr<SimpleOscGridUI> _gridUI;
    std::unique_ptr<SimpleOscUnitUI> _unitUI;

};

}