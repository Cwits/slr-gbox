// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"
#include "ui/display/primitives/Button.h"

#include <vector>

namespace UI {

class UIContext;

struct VirtualMidiKeyboard : public Popup {
    VirtualMidiKeyboard(BaseWidget * parent, UIContext * const uictx);
    ~VirtualMidiKeyboard();

    const int octaveModifier() const { return _octave; }

    private:
    int _octave;

    struct Key : public Button {
        Key(BaseWidget *parent, std::string text = "");
        ~Key();

        int note;
    };

    std::vector<Key*> _keys;
};

}