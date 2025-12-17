// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/Button.h" 
#include "ui/display/primitives/DropDown.h"

#include "defines.h"

namespace slr {
    class AudioUnitView;
    class AudioRoute;
    class MidiRoute;
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

    struct Route {
        Button * _extint; //button
        DropDown * _dropdown; //dropdown
        Button * _channelMap; //dropdown
        Button * _addRemoveButton; //button
    };

    struct AudioTab : public BaseWidget {
        AudioTab(BaseWidget *parent, const slr::ID &id);
        ~AudioTab();

        void update();
        void forcedClose();

        private:
        const slr::ID & _currentId;

        void addAsInput(bool isNew, const slr::AudioRoute &in, const int posY);
        void addAsOutput(bool isNew, const slr::AudioRoute &out, const int posY);
        
        std::vector<Route> _inputs;
        std::vector<Route> _outputs;

        Route _nextInput;
        Route _nextOutput;
        
        void newRoute(bool isInput);
    };

    struct MidiTab : public BaseWidget {
        MidiTab(BaseWidget *parent, const slr::ID &id);
        ~MidiTab();

        void update();
        void forcedClose();

        private:
        const slr::ID & _currentId;

        void addAsInput(bool isNew, const slr::MidiRoute &in, const int posY);
        void addAsOutput(bool isNew, const slr::MidiRoute &out, const int posY);

        std::vector<Route> _inputs;
        std::vector<Route> _outputs;

        Route _nextInput;
        Route _nextOutput;

        void newRoute(bool isInput);
    };

    Label * _text;
    Label * _inputsText;
    Label * _outputsText;

    Button * _btnAudioTab;
    Button * _btnMidiTab;
    Button * _btnApply;

    AudioTab * _audioTab;
    MidiTab * _midiTab;
    
    void forcedClose() override;
};

}