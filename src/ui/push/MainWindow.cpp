// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/MainWindow.h"

#include "push/PushPainter.h"
#include "logger.h"

namespace PushUI {

int clrRoot = 50;
int clrIn = 20;
int clrOut = 30;
int clrPress = 10;

MainWindow::MainWindow(PushLib::PushContext * const pctx) :
    PushLib::Widget(nullptr, pctx)
{
}

MainWindow::~MainWindow() {

}

void MainWindow::paint(PushLib::Painter &paint) {
    using namespace PushLib;
    // paint.clear();
    paint.fill(0);

    paint.filledRectangle(RectX, 50+RectY, 50, 50, COLORS::Red);
    paint.Rectangle(70+RectX, 50+RectY, 50, 50, 1, COLORS::Green);
    paint.filledRectangle(140+RectX, 50+RectY, 50, 50, rgb(255, 100, 100));
    uint16_t color = 0;
    if(RectColor == 0) color = rgb(255, 0, 0);
    else if(RectColor == 1) color = rgb(0, 255, 0);
    else if(RectColor == 2) color = rgb(0, 0, 255);
    else if(RectColor == 3) color = rgb(255, 255, 0);
    else if(RectColor == 4) color = rgb(255, 0, 255);
    else if(RectColor == 5) color = rgb(0, 255, 255);
    else if(RectColor == 6) color = rgb(100, 100, 100);
    else if(RectColor == 7) color = rgb(255, 255, 255);
    paint.Rectangle(500+RectX, 50+RectY, 50, 50, 1, color);
    paint.writeString(10, 120, "Hello wirdl", Font_16x26, COLORS::White);
}

bool MainWindow::handleButton(PushLib::ButtonEvent &ev) {
    using namespace PushLib;
    LOG_INFO("Button %d %s event", static_cast<int>(ev.button), 
                (static_cast<int>(ev.type) ? "pressed" : "released"));

        //... do some logic?
        if(ev.button == Button::Left) {
            if(ev.type == ButtonEventType::Pressed)
                RectX -= 10;
        } else if(ev.button == Button::Right) {
            if(ev.type == ButtonEventType::Pressed) 
                RectX += 10;
        } else if(ev.button == Button::Up) {
            if(ev.type == ButtonEventType::Pressed)
                RectY -= 10;
        } else if(ev.button == Button::Down) {
            if(ev.type == ButtonEventType::Pressed)
                RectY += 10;
        } else if(ev.button == Button::DisplayBottom1) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 0;
        } else if(ev.button == Button::DisplayBottom2) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 1;
        } else if(ev.button == Button::DisplayBottom3) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 2;
        } else if(ev.button == Button::DisplayBottom4) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 3;
        } else if(ev.button == Button::DisplayBottom5) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 4;
        } else if(ev.button == Button::DisplayBottom6) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 5;
        } else if(ev.button == Button::DisplayBottom7) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 6;
        } else if(ev.button == Button::DisplayBottom8) {
            if(ev.type == ButtonEventType::Pressed)
                RectColor = 7;
        } else if(ev.button == Button::Accent) {
            if(ev.type == ButtonEventType::Pressed) {
                _pctx->setPadsColors(clrRoot, clrIn, clrOut, clrPress);
                clrRoot++;
                if(clrRoot > 127) clrRoot = 0;
                clrIn++;
                if(clrIn > 127) clrIn = 0;
                clrOut++;
                if(clrOut > 127) clrOut = 0;
                clrPress++;
                if(clrPress > 127) clrPress = 0;
            }
        } else if(ev.button == Button::AddDevice) {
            if(ev.type == ButtonEventType::Pressed) {
                _pctx->setPadsChromatic(!_pctx->isPadsChromatic());
            }
        } else if(ev.button == Button::Browser) {
            if(ev.type == ButtonEventType::Pressed) {
                static int tmp = 0;
                _pctx->setPadsScale(static_cast<Scales>(tmp));
                tmp++;
                if(tmp >= static_cast<int>(Scales::LAST)) tmp = 0;
            }
        } else if(ev.button == Button::Device) {
            if(ev.type == ButtonEventType::Pressed) {
                static int tmp2 = 0;
                _pctx->setPadsLayout(static_cast<LayoutStyle>(tmp2));
                if(tmp2 == 0) tmp2 = 1;
                else if(tmp2 == 1) tmp2 = 0;
            }
        }
    return true;
}

}