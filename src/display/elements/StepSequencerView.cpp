// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/elements/StepSequencerView.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"
#include "display/primitives/DropDown.h"

#include "display/popups/PopupManager.h"

#include "display/utility/UIContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/DefaultStyles.h"

#include "snapshots/SequenceView.h"
#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include  "core/actions/Actions.h"
#include "common/uiutility.h"
#include "common/logger.h"

#include <cmath>

namespace UI {

const std::string_view notarget = "No Sequence Target";

StepSequencerView::StepSequencerView(BaseWidget * parent, UIContext * const uictx) :
    View(parent, uictx),
    _container(this)
{
    setPos(Layout::STEP_SEQ_X, Layout::STEP_SEQ_Y);
    setSize(Layout::STEP_SEQ_WIDTH, Layout::STEP_SEQ_HEIGHT);
    addStyle(&Style::workspace);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);

    /* First Line */
    _lblNumOfSequencesText = std::make_unique<Label>(this, "Sequence:");
    _lblNumOfSequencesText->setPos(10, 30);
    _lblNumOfSequencesText->setSize(150, 40);

    _lblNumOfSequences = std::make_unique<Label>(this, "0 / 0");
    _lblNumOfSequences->setPos(180, 30);
    _lblNumOfSequences->setSize(120, 40);

    BUTTONDEF(_btnPrevSequence, 300, 10, "<");
    _btnPrevSequence->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        if(this->_currentVisibleSeq > 0) {
            this->_currentVisibleSeq--;
            this->showByPos(this->_currentVisibleSeq);
        }
        return true;
    });

    BUTTONDEF(_btnNextSequence, 390, 10, ">");
    _btnNextSequence->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        if(this->_currentVisibleSeq < this->_sequences.size()-1) {
            this->_currentVisibleSeq++;
            this->showByPos(this->_currentVisibleSeq);
        }
        return true;
    });

    BUTTONDEF(_btnNewSequence, 1400, 10, "+");
    _btnNewSequence->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::CreateNewSequence>();
        slr::EmitAction(std::move(act));
        return true;
    });

    BUTTONDEF(_btnDeleteSequence, 1490, 10, LV_SYMBOL_TRASH);
    _btnDeleteSequence->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        return true;
    });

    _lblModeText = std::make_unique<Label>(this, "Sequencing");
    _lblModeText->setSize(180, 40);
    _lblModeText->setPos(1690, 30);

    BUTTONDEF(_btnModeToggle, 1600, 10, "Mode");
    _btnModeToggle->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        return true;
    });

    /* Second Line */
    int y = 120;
    _lblTotalStepsText = std::make_unique<Label>(this, "Total Steps:");
    _lblTotalStepsText->setSize(200, 40);
    _lblTotalStepsText->setPos(10, y);

    _lblTotalSteps = std::make_unique<Label>(this, "NaN");
    _lblTotalSteps->setSize(70, 40);
    _lblTotalSteps->setPos(200, y);

    _lblStepLengthText = std::make_unique<Label>(this, "Step Length:");
    _lblStepLengthText->setSize(200, 40);
    _lblStepLengthText->setPos(330, y);

    _lblStepLength = std::make_unique<Label>(this, "NaN");
    _lblStepLength->setSize(70, 40);
    _lblStepLength->setPos(540, y);

    _lblPageText = std::make_unique<Label>(this, "Page:");
    _lblPageText->setSize(100, 40);
    _lblPageText->setPos(800, y);

    _lblPage = std::make_unique<Label>(this, "0 / 0");
    _lblPage->setSize(100, 40);
    _lblPage->setPos(890, y);

    BUTTONDEF(_btnTargetManager, 1600, 100, "Targets");
    _btnTargetManager->setSize(Layout::Button*2, Layout::Button);
    _btnTargetManager->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_tpop->clear();
        this->_uictx->_popManager->enableTargetSelectPopup();
        return true;
    });

    //page buttons
    int tmpx = 20;
    int maxPages = slr::EVENTS_COUNT/slr::STEPS_PER_PAGE;
    for(int i=0; i<maxPages; ++i) {
        std::unique_ptr<Button> btn = std::make_unique<Button>(this, "");
        btn->setSize(Layout::Button, Layout::Button);
        btn->setPos(tmpx, 190);
        btn->tapCallback([idx = i, this](const GestLib::TapGesture &tap) -> bool {
            this->_currentPage = idx;
            this->switchPage(this->_currentPage);
            return true;
        });
        btn->hide();
        _pageButtons.push_back(std::move(btn));
        tmpx += 90;
    }

    BUTTONDEF(_btnNewPage, 1600, 190, "+");
    _btnNewPage->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        const SequenceUI *sui = this->currentSequence();
        if(!sui) return false;

        auto act = std::make_unique<slr::Actions::ModifySequence>();
        act->sequenceId = sui->view()->id();
        act->duration = sui->view()->stepDuration();
        act->stepCount = sui->view()->stepCount() + slr::STEPS_PER_PAGE;
        slr::EmitAction(std::move(act));
        return true;
    });
    
    BUTTONDEF(_btnDeletePage, 1690, 190, LV_SYMBOL_TRASH);
    _btnDeletePage->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        const SequenceUI *sui = this->currentSequence();
        if(!sui) return false;

        auto act = std::make_unique<slr::Actions::ModifySequence>();
        act->sequenceId = sui->view()->id();
        act->duration = sui->view()->stepDuration();
        act->stepCount = sui->view()->stepCount() - slr::STEPS_PER_PAGE;
        slr::EmitAction(std::move(act));
        return true;
    });
}

StepSequencerView::~StepSequencerView() {

}

void StepSequencerView::pollUIUpdate() {
    static char slowmedown = 0; //have to figure some other way to update ui...
    slowmedown++;
    if(slowmedown == 15) {
        slowmedown = 0;
        _tpop->update();
    }

    SequenceUI *sc = currentSequence();
    if(sc) {
        const slr::SequenceView *sv = sc->view();
        uint64_t v = sv->version();
        if(sc->_uiVersion == v) return;
        sc->_uiVersion = v;

        unsigned int totalSteps = sv->stepCount();

        //manage texts
        _lblTotalSteps->setText(std::to_string(totalSteps));
        _lblStepLength->setText(std::to_string(static_cast<int>(sv->stepDuration())));
        
        int pages = totalSteps / slr::STEPS_PER_PAGE; //16 - fixed size of steps per page
        std::string text;
        text.clear();
        text.append(std::to_string(_currentPage+1));
        text.append(" / ");
        text.append(std::to_string(pages));
        _lblPage->setText(text);

        for(int i=0; i<_pageButtons.size(); ++i) _pageButtons[i]->hide();
        //manage pages
        for(int i=0; i<pages; ++i) {
            _pageButtons[i]->show();
        }

        for(int i=0; i<_pageButtons.size(); ++i) _pageButtons[i]->setColor(Colors::ButtonReleased);
        _pageButtons[_currentPage]->setColor(Colors::Green);

        //manage layers
        int layers = sv->activeLayerCount();
        int viewCount = _container.activeLayers();
        
        slr::Color unique = sv->color();
        if(layers != viewCount) {
            for(int i=0; i<slr::LAYERS_COUNT; ++i) _container._layerView[i].hide();

            //prepare
            for(int i=0; i<layers; ++i) {
                const slr::LayerView lv = sv->layer(i);
                SequenceLayer &l = _container._layerView[i];
                for(int i=0; i<16; ++i) {
                    int eventIdx = (this->_currentPage*slr::STEPS_PER_PAGE) + i;
                    //update buttons
                    if(lv._events[eventIdx]) {
                        //color the button
                        l._steps[i]->setColor(lv_color_make(unique.r, unique.g, unique.b));
                    } else {
                        //default color
                        l._steps[i]->setColor(Colors::ButtonReleased);
                    }
                }

                if(lv._mute) l._btnMute->setColor(lv_color_make(0, 0, 255)); //color mute btn
                else l._btnMute->setColor(Colors::ButtonReleased);
                
                l._isActive = true;
            }

            if(layers == 16) {
                _container._btnAddLayer->hide();
                _container._btnRemoveLayer->show();
            } else if(layers == 0) {
                _container._btnAddLayer->show();
                _container._btnRemoveLayer->hide();
            } else {
                _container._btnAddLayer->show();
                _container._btnRemoveLayer->show();
            }

            for(int i=0; i<layers; ++i) _container._layerView[i].show();
        } else {
            //update only visible buttons
            for(int i=0; i<layers; ++i) {
                const slr::LayerView lv = sv->layer(i);
                SequenceLayer &l = _container._layerView[i];
                for(int i=0; i<16; ++i) {
                    int eventIdx = (this->_currentPage*slr::STEPS_PER_PAGE) + i;
                    //update buttons
                    if(lv._events[eventIdx]) {
                        //color the button
                        l._steps[i]->setColor(lv_color_make(unique.r, unique.g, unique.b));
                    } else {
                        //default color
                        l._steps[i]->setColor(Colors::ButtonReleased);
                    }
                }

                if(lv._mute) l._btnMute->setColor(lv_color_make(0, 0, 255)); //color mute btn
                else l._btnMute->setColor(Colors::ButtonReleased);
                
                l._isActive = true;
            }
        }
            
        int btnY = (layers * (Layout::Button-10+10)) + 30;
        _container._btnAddLayer->setPos(_container._btnAddLayer->getX(), btnY);
        _container._btnRemoveLayer->setPos(_container._btnRemoveLayer->getX(), btnY);
    }
}

void StepSequencerView::createSequenceUI(const std::shared_ptr<slr::SequenceView> view) {
    _sequences.push_back(std::make_unique<SequenceUI>(view));
    showByPos(_sequences.size()-1);
}

SequenceUI * StepSequencerView::currentSequence() const {
    if(_sequences.size() == 0) return nullptr;
    // assert(_currentVisibleSeq >= 1);
    return _sequences[this->_currentVisibleSeq].get();
}

//from [0 to size)
void StepSequencerView::showByPos(std::size_t pos) {
    if(_sequences.size() == 0) return;

    // hideAllSeq();

    // _sequences[pos]->show();  

    _currentVisibleSeq = pos;

    std::string text;
    text.append(std::to_string(_currentVisibleSeq+1));
    text.append(" / ");
    text.append(std::to_string(_sequences.size()));

    _lblNumOfSequences->setText(text);

    const slr::SequenceView *sv = currentSequence()->view();
    unsigned int totalSteps = sv->stepCount();

    //manage texts
    _lblTotalSteps->setText(std::to_string(totalSteps));
    _lblStepLength->setText(std::to_string(static_cast<int>(sv->stepDuration())));
    
    int pages = totalSteps / slr::STEPS_PER_PAGE; //16 - fixed size of steps per page
    text.clear();
    text.append("1 / ");
    text.append(std::to_string(pages));
    _lblPage->setText(text);

    _currentPage = 0;

    for(int i=0; i<_pageButtons.size(); ++i) _pageButtons[i]->hide();
    //manage pages
    for(int i=0; i<pages; ++i) {
        _pageButtons[i]->show();
    }

    for(int i=0; i<_pageButtons.size(); ++i) _pageButtons[i]->setColor(Colors::ButtonReleased);
    _pageButtons[0]->setColor(Colors::Green);
    //_pageButtons

    //manage layers
    int layers = sv->activeLayerCount();
    int viewCount = _container.activeLayers();
    slr::Color unique = sv->color();
    if(layers != viewCount) {
        for(int i=0; i<slr::LAYERS_COUNT; ++i) _container._layerView[i].hide();

        //prepare
        for(int i=0; i<layers; ++i) {
            const slr::LayerView lv = sv->layer(i);
            SequenceLayer &l = _container._layerView[i];
            for(int i=0; i<16; ++i) {
                //update buttons
                if(lv._events[i]) {
                    //color the button
                    l._steps[i]->setColor(lv_color_make(unique.r, unique.g, unique.b));
                } else {
                    //default color
                    l._steps[i]->setColor(Colors::ButtonReleased);
                }
            }

            if(lv._mute) l._btnMute->setColor(lv_color_make(0, 0, 255)); //color mute btn
            else l._btnMute->setColor(Colors::ButtonReleased);
            
            l._isActive = true;
        }

        for(int i=0; i<layers; ++i) _container._layerView[i].show();
    }

    if(layers == 16) {
        _container._btnAddLayer->hide();
        _container._btnRemoveLayer->show();
    } else if(layers == 0) {
        _container._btnAddLayer->show();
        _container._btnRemoveLayer->hide();
    } else {
        _container._btnAddLayer->show();
        _container._btnRemoveLayer->show();
    }

    int btnY = (layers * (Layout::Button-10+10)) + 30; //if swiped than wrong number
    _container._btnAddLayer->setPos(_container._btnAddLayer->getX(), btnY);
    _container._btnRemoveLayer->setPos(_container._btnRemoveLayer->getX(), btnY);
}

//[0, idx)
void StepSequencerView::switchPage(int idx) {
    const slr::SequenceView *sv = currentSequence()->view();
    unsigned int totalSteps = sv->stepCount();
    int pages = totalSteps / slr::STEPS_PER_PAGE;

    if(idx+1 > pages) return;

    //update buttons on active layers
    int layers = sv->activeLayerCount();
    slr::Color unique = sv->color();
    for(int l=0; l<layers; ++l) {
        SequenceLayer &sl = _container._layerView[l];
        const slr::LayerView &lv = sv->layer(l);

        for(int b=0; b<slr::STEPS_PER_PAGE; ++b) {
            int btnidx = (idx*slr::STEPS_PER_PAGE)+b;
            if(lv._events[btnidx]) {
                //color the button
                sl._steps[b]->setColor(lv_color_make(unique.r, unique.g, unique.b));
            } else {
                //default color
                sl._steps[b]->setColor(Colors::ButtonReleased);
            }
        }

        sl._lblNote->setText(std::to_string(lv._note));
    }

    std::string text;
    text.append(std::to_string(idx+1));
    text.append(" / ");
    text.append(std::to_string(pages));
    _lblPage->setText(text);

    //ugh, this is weird thing with default color, feels more like some hack
    for(int i=0; i<_pageButtons.size(); ++i) { 
        _pageButtons[i]->setDefaultColor(Colors::ButtonReleased); 
        _pageButtons[i]->setColor(Colors::ButtonReleased);
    }
    _pageButtons[idx]->setDefaultColor(Colors::Green); //this happens before touch up callback
}

SequenceUI::SequenceUI(const std::shared_ptr<slr::SequenceView> view) :
    _view(view),
    _uiVersion(0)
{
}

const slr::ID SequenceUI::id() const {
    return _view->id();
}

void StepSequencerView::SequenceLayer::hide() {
    for(auto &b : _steps) b->hide();
    _btnMute->hide();
    _lblNote->hide();
    _isActive = false;
}

void StepSequencerView::SequenceLayer::show() {
    for(auto &b : _steps) b->show();
    _btnMute->show();
    _lblNote->show();
}

void StepSequencerView::SequenceLayer::moveY(int amount) {
    for(auto &b : _steps) b->setPos(b->getX(), b->getY()+amount);
    _btnMute->setPos(_btnMute->getX(), _btnMute->getY()+amount);
    _lblNote->setPos(_lblNote->getX(), _lblNote->getY()+amount);
}

StepSequencerView::LayersContainer::LayersContainer(StepSequencerView *parent) :
    BaseWidget(parent, true),
    _view(parent)
{
    setPos(0, 290);
    setSize(Layout::STEP_SEQ_WIDTH, Layout::STEP_SEQ_HEIGHT-290); //overlaps with bottom panel
    setColor(lv_color_hex(0xac857e));
    // setColor(lv_color_make(rand()%255, rand()%255, rand()%255));
    
    int ypos = 10;
    int size = Layout::Button-10;
    for(int i=0; i<slr::LAYERS_COUNT; ++i) {
        StepSequencerView::SequenceLayer l;

        int xpos = 30;
        for(int z=0; z<16; ++z) {
            std::unique_ptr<Button> btn = std::make_unique<Button>(this, "");
            btn->setSize(size, size);
            btn->setPos(xpos, ypos);
            btn->tapCallback([this, buttonIdx = z, layerIdx = i](const GestLib::TapGesture &tap) -> bool {
                const SequenceUI *sui = this->_view->currentSequence();
                if(sui == nullptr) return false; //just in case, but there should be no buttons visible

                const slr::LayerView &lv = sui->view()->layer(layerIdx);
                const int eventIdx = (this->_view->_currentPage*slr::STEPS_PER_PAGE) + buttonIdx;
                // slr::MidiEvent mev = sui->view()->getEvent(layerIdx, eventIdx);

                auto act = std::make_unique<slr::Actions::ModifySequenceEvent>();
                act->sequenceId = this->_view->currentSequence()->view()->id();
                act->layer = layerIdx;
                act->eventNum = eventIdx;
                act->enabled = lv._events[eventIdx] ? false : true;
                // act->note = mev.note;
                // act->velocity = mev.velocity;
                slr::EmitAction(std::move(act));
                return true;
            });
            // l._steps.push_back(std::move(btn));
            btn->hide();
            l._steps[z] = std::move(btn);
            xpos += (size+10);
        }

        l._btnMute = std::make_unique<Button>(this, "Mute");
        l._btnMute->setSize(size, size);
        l._btnMute->setPos(xpos+200, ypos);
        l._btnMute->hide();

        l._lblNote = std::make_unique<Label>(this, std::to_string(36+i));
        l._lblNote->setSize(300, 40);
        l._lblNote->setPos(xpos+300+size+10, ypos+20);
        l._lblNote->tapCallback([self = this, layer = i, ssview = _view, ptr = l._lblNote.get()](const GestLib::TapGesture &tap) -> bool {
            //small popup for edit note, velocity, and smth else
            ssview->_uictx->_popManager->enableKeyboard(
                ptr->text(),
                [self, layer, ssview](const std::string &text) {
                    if(text.empty()) return;
                    SequenceUI * current = ssview->currentSequence();
                    if(!current) return;

                    int res = std::stoi(text);
                    if(res > 0 && res < 128) {

                        auto act = std::make_unique<slr::Actions::ModifySequenceLayer>();

                        act->sequenceId = current->id();
                        act->layer = layer;
                        act->note = res;
                        slr::EmitAction(std::move(act));
                    }
                }
            );
            return true;
        });
        l._lblNote->hide();

        l._isActive = false;

        _layerView[i] = std::move(l);
        ypos += (size+10);
    }

    _btnAddLayer = std::make_unique<Button>(this, LV_SYMBOL_PLUS);
    _btnAddLayer->setSize(Layout::Button, Layout::Button);
    _btnAddLayer->setPos(Layout::STEP_SEQ_WIDTH/2, Layout::STEP_SEQ_HEIGHT-290-Layout::Button-10);
    _btnAddLayer->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        const SequenceUI *sui = this->_view->currentSequence();
        if(!sui) return false;

        int activeLayers = 0; //sui->view()->activeLayerCount();
        for(int i=0; i<16; ++i) {
            const slr::LayerView &lv = sui->view()->layer(i);
            if(lv._active) activeLayers++;
        }

        if(activeLayers == 16) return false; 

        auto act = std::make_unique<slr::Actions::ModifySequenceLayer>();
        act->sequenceId = sui->view()->id();
        act->layer = activeLayers;
        act->active = true;
        act->mute = false;
        slr::EmitAction(std::move(act));
        return true;
    });
    _btnAddLayer->hide();

    _btnRemoveLayer = std::make_unique<Button>(this, LV_SYMBOL_MINUS);
    _btnRemoveLayer->setSize(Layout::Button, Layout::Button);
    _btnRemoveLayer->setPos(Layout::STEP_SEQ_WIDTH/2+Layout::Button+10, Layout::STEP_SEQ_HEIGHT-290-Layout::Button-10);
    _btnRemoveLayer->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        const SequenceUI *sui = this->_view->currentSequence();
        if(!sui) return false;

        int lastActiveIdx = 0;
        bool noActive = true;
        for(int i=0; i<16; ++i) {
            const slr::LayerView &lv = sui->view()->layer(i);
            if(lv._active) {
                lastActiveIdx = i;
                noActive = false;
            }
        }

        if(noActive) return false;

        auto act = std::make_unique<slr::Actions::ModifySequenceLayer>();
        act->sequenceId = sui->view()->id();
        act->layer = lastActiveIdx;
        act->active = false;
        slr::EmitAction(std::move(act));
        return true;
    });
    _btnRemoveLayer->hide();

    // _flags.isSwipe = true;
    swipeCallback(std::bind(&StepSequencerView::LayersContainer::handleSwipe, this, std::placeholders::_1));

    show();
}

int StepSequencerView::LayersContainer::activeLayers() const {
    int ret = 0;
    for(int i=0; i<slr::LAYERS_COUNT; ++i) {
        if(_layerView[i]._isActive) ret++;
    }
    return ret;
}

bool StepSequencerView::LayersContainer::handleSwipe(const GestLib::SwipeGesture & swipe) {
    if(swipe.state == GestLib::GestureState::Start) {

    } else if(swipe.state == GestLib::GestureState::Move) {
        if(swipe.dy > 0) {
            //scroll up (finger move from top to bottom)
            
            //y min == 10
            int y = _layerView[0]._btnMute->getY();
            if(y < 10) {
                //we can scroll up
                int diff = y + swipe.dy;
                int toScroll = 0;
                if(diff > swipe.dy) {
                    toScroll = diff - swipe.dy;
                } else if(diff < swipe.dy) {
                    toScroll = swipe.dy;
                }

                if(toScroll) {
                    for(int i=0; i<16; ++i) {
                        _layerView[i].moveY(toScroll);
                    }
                    _btnAddLayer->setPos(_btnAddLayer->getX(), _btnAddLayer->getY()+toScroll);
                    _btnRemoveLayer->setPos(_btnRemoveLayer->getX(), _btnRemoveLayer->getY()+toScroll);
                }
            }
        } else if(swipe.dy < 0) {
            //scroll down (finger move from bottom to top)
            //y max == Layout::STEP_SEQ_HEIGHT-290
            int y = _btnAddLayer->getY();
            constexpr int tmp = (Layout::STEP_SEQ_HEIGHT-290)+10 - Layout::Button+10 - Layout::BOTTOM_PANEL_HEIGHT; 
            if(y > tmp) { //do i need those +10??
                int diff = y + swipe.dy;
                int toScroll = 0;
                if(diff < tmp) {
                    toScroll = y - tmp;
                } else if(diff > tmp) {
                    toScroll = swipe.dy;
                }

                if(toScroll) {
                    for(int i=0; i<16; ++i) {
                        _layerView[i].moveY(toScroll);
                    }
                    _btnAddLayer->setPos(_btnAddLayer->getX(), _btnAddLayer->getY()+toScroll);
                    _btnRemoveLayer->setPos(_btnRemoveLayer->getX(), _btnRemoveLayer->getY()+toScroll);
                }
            }
        }
    } else if(swipe.state == GestLib::GestureState::End) {

    }

    return true;
}


TargetSelectPopup::TargetSelectPopup(BaseWidget *parent, StepSequencerView *sParent, UIContext * const uictx) :
    Popup(parent, uictx),
    _view(sParent)
{
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT);
    setPos(Layout::ROUTE_MANAGER_X, Layout::ROUTE_MANAGER_Y);
    addStyle(&Style::PopupDefault);

    _lblSequenceIdText = std::make_unique<Label>(this, "Current sequence:");
    _lblSequenceIdText->setSize(300, 40);
    _lblSequenceIdText->setPos(Layout::ROUTE_MANAGER_WIDTH/2-300, 10);

    _lblSequenceId = std::make_unique<Label>(this, "NaN");
    _lblSequenceId->setSize(300, 40);
    _lblSequenceId->setPos(Layout::ROUTE_MANAGER_WIDTH/2, 10);

    _ddSelector = std::make_unique<DropDown>(this);
    _ddSelector->setPos(50, 100);
    _ddSelector->setSize(1000, Layout::Button);
    _ddSelector->selectedCallback([this](std::string selected) {
        //nothing to do here?
    });
    _ddSelector->button()->touchDownCallback([this, drop = _ddSelector.get()](const GestLib::TouchDownEvent &td) -> bool {
        drop->button()->setColor(Colors::ButtonPressed);
        
        //gather all available units
        std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
        std::vector<std::string> items;

        for(const auto *v : list) {
            items.push_back(v->name());
        }

        //exclude thos that already in targets
        SequenceUI *sc = this->_view->currentSequence();
        if(sc) {
            std::array<slr::ID, slr::TARGET_COUNT> targets = sc->view()->targets();

            for(auto it = items.begin(); it != items.end();) {
                bool erased = false;

                for(int i=0; i<slr::TARGET_COUNT; ++i) {
                    if(targets[i] == 0) continue;
                    else {
                        slr::AudioUnitView *unit = slr::ProjectView::getProjectView().getUnitById(targets[i]);
                        if(!unit) continue;

                        if(it->compare(unit->name()) == 0) {
                            it = items.erase(it);
                            erased = true; //because items.erase() can return items.end(), and we get ub after ++ in for
                            break;
                        }
                    }
                }

                if(!erased) { ++it; }
            }
        }

        drop->setItems(items);
        return true;
    });

    _btnAddTarget = std::make_unique<Button>(this, LV_SYMBOL_PLUS);
    _btnAddTarget->setPos(1400, 100);
    _btnAddTarget->setSize(Layout::Button, Layout::Button);
    _btnAddTarget->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        SequenceUI *sc = this->_view->currentSequence();
        if(!sc) return false;
        const std::string &name = this->_ddSelector->selectedItem();

        slr::AudioUnitView *view = slr::ProjectView::getProjectView().findUnitByName(name);
        if(!view) return false;

        auto act = std::make_unique<slr::Actions::ModifySequenceTarget>();
        act->sequenceId = sc->view()->id();
        act->addTarget = true;
        act->targetId = view->id();
        slr::EmitAction(std::move(act));

        this->_ddSelector->button()->setText("Select Target");
        return true;
    });

    _lastTargetCount = 0;
}

TargetSelectPopup::~TargetSelectPopup() {

}

void TargetSelectPopup::update() {
    SequenceUI *curr = _view->currentSequence();
    if(!curr) {
        _lblSequenceId->setText("NaN");
        _targets.clear();
        return;
    }

    _lblSequenceId->setText(std::to_string(curr->view()->id()));
    
    int targetCount = 0;
    std::array<slr::ID, slr::TARGET_COUNT> targets = curr->view()->targets();
    for(int i=0; i<slr::TARGET_COUNT; ++i) {
        if(targets[i] != 0) targetCount++;
    }

    if(targetCount == _lastTargetCount) return;
    _lastTargetCount = targetCount;

    _targets.clear();
    int ypos = 100;
    for(int i=0; i<slr::TARGET_COUNT; ++i) {
        if(targets[i] == 0) continue;

        slr::AudioUnitView *v = slr::ProjectView::getProjectView().getUnitById(targets[i]);
        if(!v) {
            LOG_ERROR("Failed to find unit %u", targets[i]);
            continue;
        }

        Target tg;
        tg._lblName = std::make_unique<Label>(this, v->name());
        tg._lblName->setPos(50, ypos);
        tg._lblName->setSize(1000, Layout::Button);

        tg._btnRemoveTarget = std::make_unique<Button>(this, LV_SYMBOL_MINUS);
        tg._btnRemoveTarget->setSize(Layout::Button, Layout::Button);
        tg._btnRemoveTarget->setPos(1400, ypos);
        tg._btnRemoveTarget->tapCallback([sqid = curr->view()->id(), tgid = targets[i]](const GestLib::TapGesture &tap) -> bool {
            auto act = std::make_unique<slr::Actions::ModifySequenceTarget>();
            act->sequenceId = sqid;
            act->addTarget = false;
            act->targetId = tgid;
            slr::EmitAction(std::move(act));
            return true;
        });
        
        _targets.push_back(std::move(tg));
        ypos += 100;
    }

    ypos = (100 + ((100+10) * _targets.size()));
    _ddSelector->setPos(50, ypos);
    _btnAddTarget->setPos(1400, ypos);
    //fetch targets, update drop down menu...
}


}
