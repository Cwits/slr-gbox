// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"
#include "ui/push/PushUIContext.h"

#include <memory>
#include <map>

namespace PushLib {
    class Painter;
}

namespace PushUI {

class PadLayoutWidget;
class GridWidget;
class ModuleWidget;
class BrowserWidget;
// class Settings;

struct MainWidget : public PushLib::Widget {
    MainWidget(PushLib::PushContext * const pctx);
    ~MainWidget();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    void switchToView(PushView view);
    void goToPreviousView();
    const PushView previousView() const;
    const PushView currentView() const;

    bool handleDefaultButton(PushLib::ButtonEvent &ev);
    bool handleDefaultEncoder(PushLib::EncoderEvent &ev);

    std::vector<PushLib::ButtonColor> buttonsColors() override;
    private:
    PushUIContext _puictx;

    std::unique_ptr<PadLayoutWidget> _padLayoutWidget;
    std::unique_ptr<GridWidget> _gridWidget;
    std::unique_ptr<ModuleWidget> _moduleWidget;
    std::unique_ptr<BrowserWidget> _browserWidget;

    PushLib::Widget * widgetFromView(PushView view);

    PushView _currentView = PushView::ERROR;
    PushView _previousView = PushView::ERROR;
    bool _viewSwitched;
    
    void colorButtons();
    static const PushLib::ButtonCallbackMap<MainWidget> _buttonsCallback;

    bool userBtnClb(PushLib::ButtonEvent &ev);
    bool deviceBtnClb(PushLib::ButtonEvent &ev);
    bool browserBtnClb(PushLib::ButtonEvent &ev);

    bool scaleSelector(PushLib::ButtonEvent &ev);
    bool layoutSelector(PushLib::ButtonEvent &ev);

    bool playButtonClb(PushLib::ButtonEvent &ev);
    bool recordButtonClb(PushLib::ButtonEvent &ev);
};

}