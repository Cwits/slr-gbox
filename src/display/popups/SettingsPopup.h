// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/Popup.h"
#include <memory>

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
    std::unique_ptr<Button> _btnGeneral;
    std::unique_ptr<Button> _btnProject;
    std::unique_ptr<Button> _btnAudio;
    std::unique_ptr<Button> _btnMidi;
    std::unique_ptr<Button> _btnUI;
    std::unique_ptr<Button> _btnSave;

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
        std::unique_ptr<DropDown> _ddAudioDriver;

        lv_obj_t * _lblSamplerateText;
        std::unique_ptr<DropDown> _ddSamplerate;

        lv_obj_t * _lblBlockSizeText;
        std::unique_ptr<DropDown> _ddBlockSize;

        lv_obj_t * _lblLatencyText;
        std::unique_ptr<Label> _lblLatency;
        
        private:
        UIContext * const _uictx;
    };

    struct MidiTab : public BaseWidget {
        MidiTab(BaseWidget * parent, UIContext * const uictx);
        ~MidiTab();
        
        void refreshDevices();
        
        private:
        UIContext * const _uictx;
        std::unique_ptr<Button> _btnRefresh;
        void clearMidiLabels();
        
        struct MidiSubdevLabel {
            MidiSubdevLabel() = default;;
            //input
            std::unique_ptr<Label> _path;
            
            std::unique_ptr<Label> _subInName;
            std::unique_ptr<Checkbox> _inputEnabled;
            
            //output
            std::unique_ptr<Label> _subOutName;
            std::unique_ptr<Checkbox> _outputEnabled;
        };

        struct MidiLabels {
            MidiLabels() = default;
            std::unique_ptr<Label> _deviceName;
            std::unique_ptr<Label> _presented;
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

    std::unique_ptr<GeneralTab> _generalTab;
    std::unique_ptr<ProjectTab> _projectTab;
    std::unique_ptr<AudioTab> _audioTab;
    std::unique_ptr<MidiTab> _midiTab;
    std::unique_ptr<UITab> _uiTab;
};

}