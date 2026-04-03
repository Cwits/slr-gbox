// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"
#include "ui/push/PushUIContext.h"

#include "defines.h"

#include <memory>

namespace PushLib {
    class Painter;
}

namespace slr {
    class AudioUnitView;
    class Module;
}

namespace PushUI {

class PadLayoutWidget;
class GridWidget;
class UnitWidget;
class BrowserWidget;
// class Settings;

struct RootWidget : public PushLib::Widget {
    RootWidget(PushLib::PushContext * const pctx);
    ~RootWidget();

    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    Widget * widgetToUpdate();

    void switchToView(PushView view);
    void goToPreviousView();
    const PushView previousView() const;
    const PushView currentView() const;

    bool handleDefaultButton(PushLib::ButtonEvent &ev);
    bool handleDefaultEncoder(PushLib::EncoderEvent &ev);

    std::vector<PushLib::ButtonColor> buttonsColors() override;

    void createUI(const slr::Module * mod, slr::AudioUnitView * view);
    void destroyUI(slr::ID id);

    bool hasAnythingDirty() const override;

    static RootWidget * inst();

    private:
    PushUIContext _puictx;

    std::unique_ptr<PadLayoutWidget> _padLayoutWidget;
    std::unique_ptr<GridWidget> _gridWidget;
    std::unique_ptr<UnitWidget> _unitWidget;
    std::unique_ptr<BrowserWidget> _browserWidget;

    PushLib::Widget * widgetFromView(const PushView view) const;

    PushView _currentView = PushView::ERROR;
    PushView _previousView = PushView::ERROR;
    bool _viewSwitched;
    
    void colorButtons();
    static const PushLib::ButtonCallbackMap<RootWidget> _buttonsCallback;

    bool userBtnClb(PushLib::ButtonEvent &ev);
    bool deviceBtnClb(PushLib::ButtonEvent &ev);
    bool browserBtnClb(PushLib::ButtonEvent &ev);

    bool scaleSelector(PushLib::ButtonEvent &ev);
    bool layoutSelector(PushLib::ButtonEvent &ev);

    bool playButtonClb(PushLib::ButtonEvent &ev);
    bool recordButtonClb(PushLib::ButtonEvent &ev);
};

}