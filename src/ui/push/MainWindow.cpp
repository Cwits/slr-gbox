// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/MainWindow.h"

#include "push/PushPainter.h"
#include "logger.h"

namespace PushUI {

MainWindow::MainWindow() :
    PushLib::Widget(nullptr)
{
}

MainWindow::~MainWindow() {

}

void MainWindow::paint(PushLib::Painter &paint) {
    using namespace PushLib;
    // paint.clear();
    paint.fill(0);

    paint.Rectangle(RectX, 50+RectY, 50, 50, static_cast<PushLib::Color>(PushLib::COLORS::Red));
    paint.EmptyRectangle(70+RectX, 50+RectY, 50, 50, 1, static_cast<PushLib::Color>(PushLib::COLORS::Green));
    paint.Rectangle(140+RectX, 50+RectY, 50, 50, PushLib::rgb(255, 100, 100));
    uint16_t color = 0;
    if(RectColor == 0) color = rgb(255, 0, 0);
    else if(RectColor == 1) color = rgb(0, 255, 0);
    else if(RectColor == 2) color = rgb(0, 0, 255);
    else if(RectColor == 3) color = rgb(255, 255, 0);
    else if(RectColor == 4) color = rgb(255, 0, 255);
    else if(RectColor == 5) color = rgb(0, 255, 255);
    else if(RectColor == 6) color = rgb(100, 100, 100);
    else if(RectColor == 7) color = rgb(255, 255, 255);
    paint.EmptyRectangle(500+RectX, 50+RectY, 50, 50, 1, color);
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
        }
    return true;
}

}