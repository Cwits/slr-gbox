// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"

namespace UI {

class Label;
class Button;
class UIContext;
class DropDown;
class Checkbox;

struct SettingsPopup : public Popup {
    SettingsPopup(BaseWidget * parent, UIContext * const uictx);
    ~SettingsPopup();

    void update();

    private:
    Button * _btnGeneral;
    Button * _btnProject;
    Button * _btnAudio;
    Button * _btnMidi;
    Button * _btnUI;
    Button * _btnSave;

    enum class Tab { 
        General,
        Project,
        Audio,
        Midi,
        UI
    };

    Tab _currentTab;
    void switchTab(Tab newTab);

    struct GeneralTab : public BaseWidget {
        GeneralTab(BaseWidget * parent, UIContext * const uictx);
        ~GeneralTab();
        
        private:
        UIContext * const _uictx;
    };

    struct ProjectTab : public BaseWidget {
        ProjectTab(BaseWidget * parent, UIContext * const uictx);
        ~ProjectTab();
        
        private:
        UIContext * const _uictx;
    };

    struct AudioTab : public BaseWidget {
        AudioTab(BaseWidget * parent, UIContext * const uictx);
        ~AudioTab();
        
        lv_obj_t * _lblAudioDriverText;
        DropDown * _ddAudioDriver;

        lv_obj_t * _lblSamplerateText;
        DropDown * _ddSamplerate;

        lv_obj_t * _lblBlockSizeText;
        DropDown * _ddBlockSize;

        lv_obj_t * _lblLatencyText;
        Label * _lblLatency;
        
        private:
        UIContext * const _uictx;
    };

    struct MidiTab : public BaseWidget {
        MidiTab(BaseWidget * parent, UIContext * const uictx);
        ~MidiTab();
        
        void refreshDevices();
        
        private:
        UIContext * const _uictx;
        Button * _btnRefresh;
        void clearMidiLabels();
        
        struct MidiSubdevLabel {
            //input
            Label * _path;
            
            Label * _subInName;
            Checkbox * _inputEnabled;
            
            //output
            Label * _subOutName;
            Checkbox * _outputEnabled;
        };

        struct MidiLabels {
            Label * _deviceName;
            Label * _presented;
            std::vector<MidiSubdevLabel> _subdevs;
        };

        std::vector<MidiLabels> _labelList;
    };

    struct UITab : public BaseWidget {
        UITab(BaseWidget * parent, UIContext * const uictx);
        ~UITab();
        
        private:
        UIContext * const _uictx;
    };

    GeneralTab * _generalTab;
    ProjectTab * _projectTab;
    AudioTab * _audioTab;
    MidiTab * _midiTab;
    UITab * _uiTab;
};

}