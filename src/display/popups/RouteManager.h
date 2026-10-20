// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "display/primitives/Popup.h"

#include "common/defines.h"

namespace slr {
    class AudioUnitView;
    class AudioRoute;
    class MidiRoute;
}

namespace UI {

class UIContext;
class Checkbox;
class Label;
class Button;
class DropDown;

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

    std::unique_ptr<Label> _lblTargetNameStatic;
    std::unique_ptr<Label> _lblInputTextStatic; 
    std::unique_ptr<Label> _lblOutputTextStatic; 

    std::unique_ptr<DropDown> _ddCurrentUnit;

    std::unique_ptr<Button> _btnAudioTab;
    std::unique_ptr<Button> _btnMidiTab;

    std::unique_ptr<Label> _lblMidiThruStatic; //static
    std::unique_ptr<Checkbox> _chbMidiThru;
    std::unique_ptr<Label> _lblOmniHwInputStatic; //static
    std::unique_ptr<Checkbox> _chbOmniHwInput;

    std::unique_ptr<AudioTab> _audioTab;
    std::unique_ptr<MidiTab> _midiTab;
    
    void forcedClose() override;
};

}