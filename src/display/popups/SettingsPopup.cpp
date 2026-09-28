// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/SettingsPopup.h"

#include "display/utility/layoutSizes.h"
#include "display/utility/defaultColors.h"
#include "display/utility/UIContext.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"
#include "display/primitives/DropDown.h"
#include "display/primitives/Checkbox.h"

#include "common/uiutility.h"

#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/MidiController.h"
#include  "core/actions/Actions.h"

#include "common/logger.h"

#include <vector>

namespace UI {

constexpr int TAB_BUTTON_W = 150;
constexpr int TAB_BUTTON_H = 120;

SettingsPopup::SettingsPopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(Layout::SETTINGS_POP_WIDTH, Layout::SETTINGS_POP_HEIGHT);
    setPos(Layout::SETTINGS_POP_X, Layout::SETTINGS_POP_Y);
    setColor(lv_color_hex(0x858585));

    int posy = 0;
    _btnGeneral = std::make_unique<Button>(this, "General");
    _btnGeneral->setFont(&DEFAULT_FONT);
    _btnGeneral->setPos(0, posy);
    _btnGeneral->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnGeneral->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->switchTab(Tab::General);
        return true;
    });

    posy += TAB_BUTTON_H;
    _btnProject = std::make_unique<Button>(this, "Project");
    _btnProject->setFont(&DEFAULT_FONT);
    _btnProject->setPos(0, posy);
    _btnProject->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnProject->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->switchTab(Tab::Project);
        return true;
    });

    posy += TAB_BUTTON_H;
    _btnAudio = std::make_unique<Button>(this, "Audio");
    _btnAudio->setFont(&DEFAULT_FONT);
    _btnAudio->setPos(0, posy);
    _btnAudio->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnAudio->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->switchTab(Tab::Audio);
        return true;
    });

    posy += TAB_BUTTON_H;
    _btnMidi = std::make_unique<Button>(this, "Midi");
    _btnMidi->setFont(&DEFAULT_FONT);
    _btnMidi->setPos(0, posy);
    _btnMidi->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnMidi->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->switchTab(Tab::Midi);
        return true;
    });

    posy += TAB_BUTTON_H;
    _btnUI = std::make_unique<Button>(this, "UI");
    _btnUI->setFont(&DEFAULT_FONT);
    _btnUI->setPos(0, posy);
    _btnUI->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnUI->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->switchTab(Tab::UI);
        return true;
    });

    _btnSave = std::make_unique<Button>(this, LV_SYMBOL_SAVE);
    _btnSave->setFont(&DEFAULT_FONT);
    _btnSave->setPos(0, Layout::SETTINGS_POP_HEIGHT-TAB_BUTTON_H);
    _btnSave->setSize(TAB_BUTTON_W, TAB_BUTTON_H);
    _btnSave->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // this->switchTab(Tab::Midi);
        return true;
    });

    _generalTab = std::make_unique<GeneralTab>(this, uictx);
    _projectTab = std::make_unique<ProjectTab>(this, uictx);
    _audioTab = std::make_unique<AudioTab>(this, uictx);
    _midiTab = std::make_unique<MidiTab>(this, uictx);
    _uiTab = std::make_unique<UITab>(this, uictx);
}

SettingsPopup::~SettingsPopup() {
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
    setSize(Layout::SETTINGS_POP_WIDTH-TAB_BUTTON_W, Layout::SETTINGS_POP_HEIGHT);
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
    setSize(Layout::SETTINGS_POP_WIDTH-TAB_BUTTON_W, Layout::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
}

SettingsPopup::ProjectTab::~ProjectTab() {

}

SettingsPopup::AudioTab::AudioTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(Layout::SETTINGS_POP_WIDTH-TAB_BUTTON_W, Layout::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
    

    const int lineHeight = lv_font_get_line_height(&DEFAULT_FONT);
    int posy = Layout::Margin;
    int textX = Layout::Margin;
    int fieldX = (Layout::Margin*2) + 200;

    _lblAudioDriverText = lv_label_create(lvhost());
    lv_label_set_text(_lblAudioDriverText, "Audio Driver:");
    lv_obj_set_size(_lblAudioDriverText, 200, lineHeight);
    lv_obj_set_pos(_lblAudioDriverText, textX, posy);
    lv_obj_set_style_text_font(_lblAudioDriverText, &DEFAULT_FONT, 0);

    std::string selected = slr::SettingsManager::getAudioDriver();
    std::vector<std::string> items;
    items.push_back("Dummy Driver"); items.push_back("Jack Driver");

    _ddAudioDriver = std::make_unique<DropDown>(this);
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
    

    posy += ((Layout::Margin*2)+lineHeight);
    _lblSamplerateText = lv_label_create(lvhost());
    lv_label_set_text(_lblSamplerateText, "Sample Rate:");
    lv_obj_set_size(_lblSamplerateText, 200, lineHeight);
    lv_obj_set_pos(_lblSamplerateText, textX, posy);
    lv_obj_set_style_text_font(_lblSamplerateText, &DEFAULT_FONT, 0);

    items.clear();
    items.push_back("44100");
    selected = std::to_string(slr::SettingsManager::getSampleRate());
    _ddSamplerate = std::make_unique<DropDown>(this);
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


    posy += (lineHeight + (Layout::Margin*2));
    _lblBlockSizeText = lv_label_create(lvhost());
    lv_label_set_text(_lblBlockSizeText, "Block Size:");
    lv_obj_set_size(_lblBlockSizeText, 200, lineHeight);
    lv_obj_set_pos(_lblBlockSizeText, textX, posy);
    lv_obj_set_style_text_font(_lblBlockSizeText, &DEFAULT_FONT, 0);

    selected = std::to_string(slr::SettingsManager::getBlockSize());
    items.clear();
    items.push_back("64"); items.push_back("128"); items.push_back("256");
    items.push_back("512"); items.push_back("1024");

    _ddBlockSize = std::make_unique<DropDown>(this);
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

    posy += (lineHeight + (Layout::Margin*2));
    
    _lblLatencyText = lv_label_create(lvhost());
    lv_label_set_text(_lblLatencyText, "Latency comp.:");
    lv_obj_set_size(_lblLatencyText, 200, lineHeight);
    lv_obj_set_pos(_lblLatencyText, textX, posy);
    lv_obj_set_style_text_font(_lblLatencyText, &DEFAULT_FONT, 0);

    _lblLatency = std::make_unique<Label>(this, std::to_string(slr::SettingsManager::getManualLatencyCompensation()));
    _lblLatency->setPos(fieldX, posy);
    _lblLatency->setSize(600, lineHeight);
    _lblLatency->setFont(&DEFAULT_FONT);
    _lblLatency->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
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
        return true;
    });
}

SettingsPopup::AudioTab::~AudioTab() {
    lv_obj_delete(_lblAudioDriverText);
    lv_obj_delete(_lblSamplerateText);
    lv_obj_delete(_lblBlockSizeText);
    lv_obj_delete(_lblLatencyText);
}

SettingsPopup::MidiTab::MidiTab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(Layout::SETTINGS_POP_WIDTH-TAB_BUTTON_W, Layout::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);
    
    _btnRefresh = std::make_unique<Button>(this, LV_SYMBOL_REFRESH);
    _btnRefresh->setPos(Layout::Margin, Layout::Margin);
    _btnRefresh->setSize(Layout::Button, Layout::Button);
    _btnRefresh->setFont(&DEFAULT_FONT);
    _btnRefresh->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->refreshDevices();
        return true;
    });
}

SettingsPopup::MidiTab::~MidiTab() {
    clearMidiLabels();
}

void SettingsPopup::MidiTab::refreshDevices() {
    slr::MidiController * ctl = slr::ControlEngine::midiController();

    std::vector<slr::MidiDevice> list = ctl->devList();

    clearMidiLabels();

    int posy = (Layout::Margin*2)+Layout::Button;
    int lineHeight = lv_font_get_line_height(&DEFAULT_FONT);
    int width = this->width();
    std::string text;
    for(slr::MidiDevice &dev : list) {
        text = dev._name;
        MidiLabels device;
        std::unique_ptr<Label> devName = std::make_unique<Label>(this, dev._name);
        devName->setPos(Layout::Margin, posy);
        devName->setSize(width/2, lineHeight);
        devName->setFont(&DEFAULT_FONT);
        device._deviceName = std::move(devName);

        std::unique_ptr<Label> presented = std::make_unique<Label>(this, (dev._online ? "presented" : "not presented"));
        presented->setPos(width/2, posy);
        presented->setSize(width/2, lineHeight);
        presented->setFont(&DEFAULT_FONT);
        device._presented = std::move(presented);

        posy += (lineHeight+Layout::Margin);
        for(slr::MidiSubdevice &sub : dev._ports) {
            const int subdevHeight = Layout::CHECKBOX;
            MidiSubdevLabel subdev;

            std::unique_ptr<Label> path = std::make_unique<Label>(this, sub._path);
            path->setPos(Layout::Margin + 30, posy);
            path->setSize(150, lineHeight);
            path->setFont(&DEFAULT_FONT);
            subdev._path = std::move(path);

            if(sub._hasInput) {
                text.clear();
                text.append("In ");
                text.append(sub._inputName);
                std::unique_ptr<Label> subInName = std::make_unique<Label>(this, text);
                subInName->setPos(Layout::Margin + 200, posy);
                subInName->setSize(width/2, lineHeight);
                subInName->setFont(&DEFAULT_FONT);
                subdev._subInName = std::move(subInName);

                std::unique_ptr<Checkbox> subInCheck = std::make_unique<Checkbox>(this);
                subInCheck->setPos(width/2 + Layout::Margin, posy);
                subInCheck->checkCallback([sub, dev](bool isChecked) mutable {
                    LOG_INFO("%s %s device is %s",
                        (&sub)->_path.c_str(), 
                        (&sub)->_inputName.c_str(),
                        (isChecked ? "enabled" : "disabled"));

                    auto act = std::make_unique<slr::Actions::ToggleMidiDevice>();
                    act->device = (&dev);
                    act->subdev = (&sub);
                    act->port = slr::DevicePort::INPUT;
                    act->newState = isChecked;
                    act->completed = [](int res) {
                        if(res == 0)
                            LOG_INFO("Midi Device toggled res %d", res);
                        else 
                            LOG_ERROR("Midi Device toggled res %d", res);
                    };
                    slr::EmitAction(std::move(act));

                });
                subdev._inputEnabled = std::move(subInCheck);
                posy += (subdevHeight+Layout::Margin);
            } else {
                subdev._subInName = nullptr;
                subdev._inputEnabled = nullptr;
            }

            if(sub._hasOutput) {
                text.clear();
                text.append("Out ");
                text.append(sub._outputName);
                std::unique_ptr<Label> subOutName = std::make_unique<Label>(this, text);
                subOutName->setPos(Layout::Margin + 200, posy);
                subOutName->setSize(width/2, lineHeight);
                subOutName->setFont(&DEFAULT_FONT);
                subdev._subOutName = std::move(subOutName);

                std::unique_ptr<Checkbox> subOutCheck = std::make_unique<Checkbox>(this);
                subOutCheck->setPos(width/2 + Layout::Margin, posy);
                subOutCheck->checkCallback([sub, dev](bool isChecked) mutable {
                    LOG_INFO("%s %s device is %s",
                        sub._path.c_str(),
                        sub._outputName.c_str(),
                        (isChecked ? "enabled" : "disabled"));

                    auto act = std::make_unique<slr::Actions::ToggleMidiDevice>();
                    act->device = (&dev);
                    act->subdev = (&sub);
                    act->port = slr::DevicePort::OUTPUT;
                    act->newState = isChecked;
                    act->completed = [](int res) {
                        if(res == 0)
                            LOG_INFO("Midi Device toggled res %d", res);
                        else 
                            LOG_ERROR("Midi Device toggled res %d", res);
                    };
                    slr::EmitAction(std::move(act));
                });
                subdev._outputEnabled = std::move(subOutCheck);
                posy += (subdevHeight+Layout::Margin);
            } else {
                subdev._subOutName = nullptr;
                subdev._outputEnabled = nullptr;
            }

            device._subdevs.push_back(std::move(subdev));
        }

        _labelList.push_back(std::move(device));
    }
}

void SettingsPopup::MidiTab::clearMidiLabels() {
    // for(auto l : _labelList) {
    //     for(auto lb : l._subdevs) {
    //         delete lb._path;

    //         if(lb._subInName) delete lb._subInName;
    //         if(lb._inputEnabled) delete lb._inputEnabled;
    //         if(lb._subOutName) delete lb._subOutName;
    //         if(lb._outputEnabled) delete lb._outputEnabled;

    //         lb._subInName = nullptr;
    //         lb._inputEnabled = nullptr;
    //         lb._subOutName = nullptr;
    //         lb._outputEnabled = nullptr;
    //         // delete lb;
    //     }
    //     delete l._deviceName;
    //     delete l._presented;

    //     l._deviceName = nullptr;
    //     l._presented = nullptr;
    //     // delete l;
    // }
    _labelList.clear();
}

SettingsPopup::UITab::UITab(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    setSize(Layout::SETTINGS_POP_WIDTH-TAB_BUTTON_W, Layout::SETTINGS_POP_HEIGHT);
    setPos(TAB_BUTTON_W, 0);
    setColor(GRAY_COLOR);

}

SettingsPopup::UITab::~UITab() {

}


}