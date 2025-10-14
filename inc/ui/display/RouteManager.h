// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/Popup.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/DropDown.h"

#include "defines.h"

namespace slr {
    class AudioUnitView;
}

namespace UI {
class UIContext;

struct RouteManager : public Popup {
    RouteManager(BaseWidget * parent, UIContext * const uictx);
    ~RouteManager();

    slr::ID _currentUnitId;

    void update();
    void liveUpdate();
    private:
    Label * _text;
    Label * _inputsText;
    Label * _outputsText;

    Button * _applyBtn;

    struct Route {
        Button * _extint; //button
        DropDown * _dropdown; //dropdown
        Button * _channelMap; //dropdown
        Button * _addRemoveButton; //button
    };

    struct ExtIO {
        enum class Dir { IN, OUT };
        Dir _direction;
        int _uniqueId;
        // std::string & _name;
    };

    std::vector<Route> _inputs;
    std::vector<Route> _outputs;

    Route _nextInput;
    Route _nextOutput;

    // std::vector<Label*> _inputs;
    // std::vector<Label*> _outputs;

    std::vector<ExtIO> _extIO;

    // lv_obj_t * _dropdownPlaceholderLeft;
    // lv_obj_t * _dropdownPlaceholderRight;
    // lv_obj_t * _channelMapPlaceholder;
    // lv_obj_t * _addPlaceholder;

    // DropDown * _dropDown;

    
    void forcedClose() override;
    bool handleTap(GestLib::TapGesture & tap) override;
    
};

}