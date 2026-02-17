// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/SettingsPopup.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"

#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/DropDown.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Checkbox.h"

#include "ui/uiutility.h"

#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/MidiController.h"
#include "logger.h"

#include <vector>

namespace UI {

constexpr int TAB_BUTTON_W = 150;
constexpr int TAB_BUTTON_H = 120;

slr::MidiPort * _testPort = nullptr;

SettingsPopup::SettingsPopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(LayoutDef::SETTINGS_POP_X, LayoutDef::SETTINGS_POP_Y);
    setColor(lv_color_hex(0x858585));

    int posy = 0;
    _btnGeneral = new Button(this, "General");
    _btnGeneral->setFont(&DEFAULT_FONT);
    _btnGeneral->setPos(0, posy);
    _btnGeneral->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnGeneral->setCallback([this]() {
        this->switchTab(Tab::General);
    });

    posy += TAB_BUTTON_H;
    _btnProject = new Button(this, "Project");
    _btnProject->setFont(&DEFAULT_FONT);
    _btnProject->setPos(0, posy);
    _btnProject->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnProject->setCallback([this]() {
        this->switchTab(Tab::Project);
    });

    posy += TAB_BUTTON_H;
    _btnAudio = new Button(this, "Audio");
    _btnAudio->setFont(&DEFAULT_FONT);
    _btnAudio->setPos(0, posy);
    _btnAudio->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnAudio->setCallback([this]() {
        this->switchTab(Tab::Audio);
    });

    posy += TAB_BUTTON_H;
    _btnMidi = new Button(this, "Midi");
    _btnMidi->setFont(&DEFAULT_FONT);
    _btnMidi->setPos(0, posy);
    _btnMidi->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnMidi->setCallback([this]() {
        this->switchTab(Tab::Midi);
    });

    posy += TAB_BUTTON_H;
    _btnUI = new Button(this, "UI");
    _btnUI->setFont(&DEFAULT_FONT);
    _btnUI->setPos(0, posy);
    _btnUI->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnUI->setCallback([this]() {
        this->switchTab(Tab::UI);
    });

    _btnSave = new Button(this, LV_SYMBOL_SAVE);
    _btnSave->setFont(&DEFAULT_FONT);
    _btnSave->setPos(0, LayoutDef::SETTINGS_POP_HEIGHT-TAB_BUTTON_H);
    _btnSave->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnSave->setCallback([this]() {
        // this->switchTab(Tab::Midi);
    });

    _generalTab = new GeneralTab(this, uictx);
    _projectTab = new ProjectTab(this, uictx);
    _audioTab = new AudioTab(this, uictx);
    _midiTab = new MidiTab(this, uictx);
    _uiTab = new UITab(this, uictx);
}

SettingsPopup::~SettingsPopup() {
    delete _generalTab;
    delete _audioTab;
    delete _midiTab;
    delete _uiTab;

    delete _btnGeneral;
    delete _btnProject;
    delete _btnAudio;
    delete _btnMidi;
    delete _btnUI;

    if(_testPort) delete _testPort;
}

void SettingsPopup::update() {
    
}

void SettingsPopup::switchTab(Tab newTab) {
    _generalTab->hide();
    _audioTab->hide();
    _midiTab->hide();
    _uiTab->hide();

    switch(newTab) {
        case(Tab::General): 
            _generalTab->show(); 
        break;
        case(Tab::Project): 
            _projectTab->show(); 
        break;
        case(Tab::Audio): 
            _audioTab->show(); 
        break;
        case(Tab::Midi): 
            _midiTab->refreshDevices();
            _midiTab->show(); 
        break;
        case(Tab::UI): 
            _uiTab->show(); 
        break;
    }
}

SettingsPopup::GeneralTab::GeneralTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH-TAB_BUTTON_W, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);

    show();
}

SettingsPopup::GeneralTab::~GeneralTab() {

}

SettingsPopup::ProjectTab::ProjectTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH-TAB_BUTTON_W, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
}

SettingsPopup::ProjectTab::~ProjectTab() {

}

SettingsPopup::AudioTab::AudioTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH-TAB_BUTTON_W, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
    

    const int lineHeight = lv_font_get_line_height(&DEFAULT_FONT);
    int posy = LayoutDef::DEFAULT_MARGIN;
    int textX = LayoutDef::DEFAULT_MARGIN;
    int fieldX = (LayoutDef::DEFAULT_MARGIN*2) + 200;

    _lblAudioDriverText = lv_label_create(lvhost());
    lv_label_set_text(_lblAudioDriverText, "Audio Driver:");
    lv_obj_set_size(_lblAudioDriverText, 200, lineHeight);
    lv_obj_set_pos(_lblAudioDriverText, textX, posy);
    lv_obj_set_style_text_font(_lblAudioDriverText, &DEFAULT_FONT, 0);

    std::string selected = slr::SettingsManager::getAudioDriver();
    std::vector<std::string> items;
    items.push_back("Dummy Driver"); items.push_back("Jack Driver");

    _ddAudioDriver = new DropDown(this);
    _ddAudioDriver->setPos(fieldX, posy+lineHeight);
    _ddAudioDriver->setSize(600, lineHeight);
    _ddAudioDriver->button()->setPos(fieldX, posy);
    _ddAudioDriver->button()->setSize(600, lineHeight);
    // _ddAudioDriver->setTextFont(&DEFAULT_FONT);
    _ddAudioDriver->setItems(items);
    _ddAudioDriver->setSelected(selected);
    _ddAudioDriver->selectedCallback([this](std::string item) {
        slr::SettingsManager::setAudioDriver(item);
        //TODO: Warning window "Please restart application"
    });
    

    posy += ((LayoutDef::DEFAULT_MARGIN*2)+lineHeight);
    _lblSamplerateText = lv_label_create(lvhost());
    lv_label_set_text(_lblSamplerateText, "Sample Rate:");
    lv_obj_set_size(_lblSamplerateText, 200, lineHeight);
    lv_obj_set_pos(_lblSamplerateText, textX, posy);
    lv_obj_set_style_text_font(_lblSamplerateText, &DEFAULT_FONT, 0);

    items.clear();
    items.push_back("44100");
    selected = std::to_string(slr::SettingsManager::getSampleRate());
    _ddSamplerate = new DropDown(this);
    _ddSamplerate->setPos(fieldX, posy+lineHeight);
    _ddSamplerate->setSize(600, lineHeight);
    _ddSamplerate->button()->setPos(fieldX, posy);
    _ddSamplerate->button()->setSize(600, lineHeight);
    _ddSamplerate->setItems(items);
    _ddSamplerate->setSelected(selected);
    _ddSamplerate->selectedCallback([this](std::string item) {
        slr::SettingsManager::setSampleRate(std::stoi(item));
        //TODO: Warning window "Please restart application"
    });


    posy += (lineHeight + (LayoutDef::DEFAULT_MARGIN*2));
    _lblBlockSizeText = lv_label_create(lvhost());
    lv_label_set_text(_lblBlockSizeText, "Block Size:");
    lv_obj_set_size(_lblBlockSizeText, 200, lineHeight);
    lv_obj_set_pos(_lblBlockSizeText, textX, posy);
    lv_obj_set_style_text_font(_lblBlockSizeText, &DEFAULT_FONT, 0);

    selected = std::to_string(slr::SettingsManager::getBlockSize());
    items.clear();
    items.push_back("64"); items.push_back("128"); items.push_back("256");
    items.push_back("512"); items.push_back("1024");

    _ddBlockSize = new DropDown(this);
    _ddBlockSize->setPos(fieldX, posy+lineHeight);
    _ddBlockSize->setSize(600, lineHeight);
    _ddBlockSize->button()->setPos(fieldX, posy);
    _ddBlockSize->button()->setSize(600, lineHeight);
    _ddBlockSize->setItems(items);
    _ddBlockSize->setSelected(selected);
    _ddBlockSize->selectedCallback([this](std::string item) {
        slr::SettingsManager::setBlockSize(std::stoi(item));
        //TODO: Warning window "Please restart application"
    });

    posy += (lineHeight + (LayoutDef::DEFAULT_MARGIN*2));
    
    _lblLatencyText = lv_label_create(lvhost());
    lv_label_set_text(_lblLatencyText, "Latency comp.:");
    lv_obj_set_size(_lblLatencyText, 200, lineHeight);
    lv_obj_set_pos(_lblLatencyText, textX, posy);
    lv_obj_set_style_text_font(_lblLatencyText, &DEFAULT_FONT, 0);

    _lblLatency = new Label(this, std::to_string(slr::SettingsManager::getManualLatencyCompensation()));
    _lblLatency->setPos(fieldX, posy);
    _lblLatency->setSize(600, lineHeight);
    _lblLatency->setFont(&DEFAULT_FONT);
    _lblLatency->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblLatency->text(),
            [this](const std::string &text) {
                if(!UIUtility::containsOnlyDigits(text)) {
                    this->_uictx->floatingText(true, "Only digits is allowed");
                    return;
                }

                int res = std::stoi(text);
                if(res < 0) {
                    this->_uictx->floatingText(true, "Compensation can't be negative");
                    return;
                }

                slr::SettingsManager::setManualLatencyCompensation(res);
                this->_lblLatency->setText(text);
            }
        );
    });
}

SettingsPopup::AudioTab::~AudioTab() {
    lv_obj_delete(_lblAudioDriverText);
    delete _ddAudioDriver;
    lv_obj_delete(_lblSamplerateText);
    delete _ddSamplerate;
    lv_obj_delete(_lblBlockSizeText);
    delete _ddBlockSize;
    lv_obj_delete(_lblLatencyText);
    delete _lblLatency;
}

SettingsPopup::MidiTab::MidiTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH-TAB_BUTTON_W, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
    
    _btnRefresh = new Button(this, LV_SYMBOL_REFRESH);
    _btnRefresh->setPos(LayoutDef::DEFAULT_MARGIN, LayoutDef::DEFAULT_MARGIN);
    _btnRefresh->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnRefresh->setFont(&DEFAULT_FONT);
    _btnRefresh->setCallback([this]() {
        this->refreshDevices();
    });
}

SettingsPopup::MidiTab::~MidiTab() {
    delete _btnRefresh;
    clearMidiLabels();
}

void SettingsPopup::MidiTab::refreshDevices() {
    slr::MidiController * ctl = slr::ControlEngine::midiController();

    std::vector<slr::MidiDevice> list = ctl->devList();

    clearMidiLabels();

    int posy = (LayoutDef::DEFAULT_MARGIN*2)+LayoutDef::BUTTON_SIZE;
    int lineHeight = lv_font_get_line_height(&DEFAULT_FONT);
    int width = this->width();
    std::string text;
    for(slr::MidiDevice &dev : list) {
        text = dev._name;
        MidiLabels device;
        Label * devName = new Label(this, dev._name);
        devName->setPos(LayoutDef::DEFAULT_MARGIN, posy);
        devName->setSize(width/2, lineHeight);
        devName->setFont(&DEFAULT_FONT);
        device._deviceName = devName;

        Label * presented = new Label(this, (dev._online ? "presented" : "not presented"));
        presented->setPos(width/2, posy);
        presented->setSize(width/2, lineHeight);
        presented->setFont(&DEFAULT_FONT);
        device._presented = presented;

        posy += (lineHeight+LayoutDef::DEFAULT_MARGIN);
        for(slr::MidiSubdevice &sub : dev._ports) {
            const int subdevHeight = LayoutDef::CHECKBOX_SIZE;
            MidiSubdevLabel subdev;

            Label * path = new Label(this, sub._path);
            path->setPos(LayoutDef::DEFAULT_MARGIN + 30, posy);
            path->setSize(150, lineHeight);
            path->setFont(&DEFAULT_FONT);
            subdev._path = path;

            if(sub._hasInput) {
                text.clear();
                text.append("In ");
                text.append(sub._inputName);
                Label * subInName = new Label(this, text);
                subInName->setPos(LayoutDef::DEFAULT_MARGIN + 200, posy);
                subInName->setSize(width/2, lineHeight);
                subInName->setFont(&DEFAULT_FONT);
                subdev._subInName = subInName;

                Checkbox * subInCheck = new Checkbox(this);
                subInCheck->setPos(width/2 + LayoutDef::DEFAULT_MARGIN, posy);
                subInCheck->setCallback([sub, dev](bool isChecked) mutable {
                    LOG_INFO("%s %s device is %s",
                        (&sub)->_path.c_str(), 
                        (&sub)->_inputName.c_str(),
                        (isChecked ? "enabled" : "disabled"));

                    slr::Events::ToggleMidiDevice e = {
                        .device = (&dev),
                        .subdev = (&sub),
                        .port = slr::DevicePort::INPUT,
                        .newState = isChecked,
                        .completed = [](int res) {
                            LOG_INFO("Midi Device toggled res %d", res);
                        }
                    };
                    slr::EmitEvent(e);

                    // if(isChecked) {
                    //     slr::MidiController * ctl = slr::ControlEngine::midiController();

                    //     if(!_testPort) {
                    //         _testPort = new slr::MidiPort;
                    //         _testPort->_path = sub._path;

                    //         ctl->openDevice(_testPort, true, false);
                    //     }
                    // }
                });
                subdev._inputEnabled = subInCheck;
                posy += (subdevHeight+LayoutDef::DEFAULT_MARGIN);
            } else {
                subdev._subInName = nullptr;
                subdev._inputEnabled = nullptr;
            }

            if(sub._hasOutput) {
                text.clear();
                text.append("Out ");
                text.append(sub._outputName);
                Label * subOutName = new Label(this, text);
                subOutName->setPos(LayoutDef::DEFAULT_MARGIN + 200, posy);
                subOutName->setSize(width/2, lineHeight);
                subOutName->setFont(&DEFAULT_FONT);
                subdev._subOutName = subOutName;

                Checkbox * subOutCheck = new Checkbox(this);
                subOutCheck->setPos(width/2 + LayoutDef::DEFAULT_MARGIN, posy);
                subOutCheck->setCallback([sub, dev](bool isChecked) mutable {
                    LOG_INFO("%s %s device is %s",
                        sub._path.c_str(),
                        sub._outputName.c_str(),
                        (isChecked ? "enabled" : "disabled"));

                    
                    slr::Events::ToggleMidiDevice e = {
                        .device = (&dev),
                        .subdev = (&sub),
                        .port = slr::DevicePort::OUTPUT,
                        .newState = isChecked,
                        .completed = [](int res) {
                            LOG_INFO("Midi Device toggled res %d", res);
                        }
                    };
                    slr::EmitEvent(e);
                });
                subdev._outputEnabled = subOutCheck;
                posy += (subdevHeight+LayoutDef::DEFAULT_MARGIN);
            } else {
                subdev._subOutName = nullptr;
                subdev._outputEnabled = nullptr;
            }

            device._subdevs.push_back(subdev);
        }

        _labelList.push_back(device);
    }
}

void SettingsPopup::MidiTab::clearMidiLabels() {
    for(auto l : _labelList) {
        for(auto lb : l._subdevs) {
            delete lb._path;

            if(lb._subInName) delete lb._subInName;
            if(lb._inputEnabled) delete lb._inputEnabled;
            if(lb._subOutName) delete lb._subOutName;
            if(lb._outputEnabled) delete lb._outputEnabled;

            lb._subInName = nullptr;
            lb._inputEnabled = nullptr;
            lb._subOutName = nullptr;
            lb._outputEnabled = nullptr;
            // delete lb;
        }
        delete l._deviceName;
        delete l._presented;

        l._deviceName = nullptr;
        l._presented = nullptr;
        // delete l;
    }
    _labelList.clear();
}

SettingsPopup::UITab::UITab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(LayoutDef::SETTINGS_POP_WIDTH-TAB_BUTTON_W, LayoutDef::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);

}

SettingsPopup::UITab::~UITab() {

}


}