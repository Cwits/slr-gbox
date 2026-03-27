// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/push/primitives/UnitUIBase.h"

#include <memory>

namespace slr {
    class TrackView;    
    class AudioUnitView;
}

namespace PushUI {

class PushUIContext;
class TrackPushUI;

struct TrackGridUI : public DefaultGridUI {
    TrackGridUI(PushLib::Widget *parent, TrackPushUI * parentUI);
    ~TrackGridUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    TrackPushUI * _parentUI;
    // uint64_t _customVersion;
    
    friend class TrackPushUI;
};

    // struct TrackModuleUI : public DefaultModuleUI {
struct TrackUnitUI : public DefaultUnitUI {
    TrackUnitUI(PushLib::Widget *parent, TrackPushUI * parentUI);
    ~TrackUnitUI();
    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;
    
    private:
    TrackPushUI * _parentUI;
    
    friend class TrackPushUI;
};


struct TrackPushUI : public UnitUIBase {
    TrackPushUI(slr::AudioUnitView * track, PushUIContext * uictx);
    ~TrackPushUI();
    
    bool create(PushUIContext * ctx) override;
    bool destroy(PushUIContext * ctx) override;

    DefaultGridUI * gridUI() override { return _gridUI.get(); }
    DefaultUnitUI * unitUI() override { return _unitUI.get(); }
    // BaseWidget * patchUI() override;
 
    private:
    slr::TrackView * const _track;

    std::unique_ptr<TrackGridUI> _gridUI;
    std::unique_ptr<TrackUnitUI> _unitUI;

};

}