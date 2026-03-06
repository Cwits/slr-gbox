// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/MainWidget.h"

#include "ui/push/PadLayoutSelector.h"

#include "push/PushPainter.h"
#include "logger.h"

namespace PushUI {

int clrRoot = 50;
int clrIn = 20;
int clrOut = 30;
int clrPress = 10;

MainWidget::MainWidget(PushLib::PushContext * const pctx) :
    PushLib::Widget(nullptr, pctx),
    _currentView(View::Grid)
{
    _padLayoutSelector = std::make_unique<PadLayoutSelector>(this, pctx);
}

MainWidget::~MainWidget() {

}

PushLib::BoundingBox MainWidget::invalidate() { //return BoundingBox of area that has to be redrawn
    PushLib::BoundingBox ret;
    if(_layoutSwitched) {
        ret.x = 0;
        ret.y = 0;
        ret.h = PushLib::DISPLAY_HEIGHT;
        ret.w = PushLib::DISPLAY_WIDTH;
        _layoutSwitched = false;
    } else {
        //invalidate?
    }
    return ret;
}

void MainWidget::paint(PushLib::Painter &painter) {
    /* 
        best case scenario - invalidate which objects are dirty and redraw only that specific area, but not whole thing...
        plus... somehow need to tell push core where bounding box changed in order for him not to recode whole screen?
        step 1:
            make invalidating of elements in drawable space
        
        step 2:
            make invalidating before preparing sendable batch in PushDisplay::updateFrame
    */

    using namespace PushLib;

    if(_currentView == View::PadLayoutSelector) {
        _padLayoutSelector->paint(painter);
    } else {
        painter.clear();
        painter.filledRectangle(RectX, 50+RectY, 50, 50, COLORS::Red);
        painter.Rectangle(70+RectX, 50+RectY, 50, 50, 1, COLORS::Green);
        painter.filledRectangle(140+RectX, 50+RectY, 50, 50, rgb(255, 100, 100));
        uint16_t color = 0;
        if(RectColor == 0) color = rgb(255, 0, 0);
        else if(RectColor == 1) color = rgb(0, 255, 0);
        else if(RectColor == 2) color = rgb(0, 0, 255);
        else if(RectColor == 3) color = rgb(255, 255, 0);
        else if(RectColor == 4) color = rgb(255, 0, 255);
        else if(RectColor == 5) color = rgb(0, 255, 255);
        else if(RectColor == 6) color = rgb(100, 100, 100);
        else if(RectColor == 7) color = rgb(255, 255, 255);
        painter.Rectangle(500+RectX, 50+RectY, 50, 50, 1, color);
        painter.writeString(10, 120, std::string("Hello wirdl"), Font_16x26, COLORS::White);
    }
}

bool MainWidget::handleButton(PushLib::ButtonEvent &ev) {
    using namespace PushLib;
    LOG_INFO("Button %d %s event", static_cast<int>(ev.button), 
                (static_cast<int>(ev.type) ? "pressed" : "released"));

    if(_currentView == View::PadLayoutSelector) {
        return _padLayoutSelector->handleButton(ev);
    } else {
        //use switch later...
    }

    if(ev.button == Button::Scale) {
        if(ev.type != ButtonEventType::Pressed) return false;

        _currentView = View::PadLayoutSelector; 
        _layoutSwitched = true;
    }

    return true;
}

bool MainWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    std::string text;
    if(ev.type == PushLib::EncoderEventType::Touched) text = "Touched";
    else if(ev.type == PushLib::EncoderEventType::Moved) text = "Moved";
    else if(ev.type == PushLib::EncoderEventType::Released) text = "Released";
    LOG_WARN("Encoder Event: %s %d %d", text.c_str(), static_cast<int>(ev.encoder), ev.delta);

    return true;
}

}