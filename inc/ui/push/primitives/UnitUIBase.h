// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"
#include "ui/push/PushUIContext.h"

#include "defines.h"

#include <memory>
#include <vector>

namespace PushLib {
    class Painter;
}

namespace slr {
    class AudioUnitView;
}

namespace PushUI {

class PushUIContext;
class DefaultGridUI;
class DefaultUnitUI;

struct UnitUIBase {
    UnitUIBase(slr::AudioUnitView *view, PushUIContext * const puictx);
    ~UnitUIBase();

    virtual bool create(PushUIContext * ctx) = 0;
    virtual bool destroy(PushUIContext * ctx) = 0;

    virtual DefaultGridUI * gridUI() = 0;
    virtual DefaultUnitUI * unitUI() = 0;

    const slr::AudioUnitView * view() const { return _view; }
    slr::AudioUnitView * view() { return _view; }

    PushUIContext * uictx() const { return _puictx; }

    private:
    slr::AudioUnitView * _view;
    PushUIContext * const _puictx;
};


class Label;
class Rectangle;

struct DefaultGridUI : public PushLib::Widget {
    DefaultGridUI(PushLib::Widget *parent, UnitUIBase * parentUI);
    ~DefaultGridUI();

    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    UnitUIBase * _parentUI;
    // std::vector<std::unique_ptr<FileView>> _fileUIs; //make the simplified version of waveform(as in messengers or smth)
    std::unique_ptr<Label> _label;
    std::unique_ptr<Rectangle> _rectangle;
};

struct DefaultUnitUI : public PushLib::Widget {
    DefaultUnitUI(PushLib::Widget *parent, UnitUIBase * parentUI);
    ~DefaultUnitUI();

    // void pollUIUpdate() override;
    void paint(PushLib::Painter &painter) override;

    private:
    UnitUIBase * _parentUI;
    
    std::unique_ptr<Label> _label;
    std::unique_ptr<Rectangle> _rectangle;
};

}