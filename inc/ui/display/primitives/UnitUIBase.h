// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Popup.h"
#include "common/Color.h"
#include "defines.h"

#include <vector>
#include <memory>

namespace slr {
    class AudioUnitView;
}

namespace UI {

class Label;
class Button;
class UIContext;
class FileView;
class DefaultGridUI;
class DefaultUnitUI;

struct UnitUIBase {
    UnitUIBase(const std::shared_ptr<const slr::AudioUnitView> &view, UIContext * uictx);
    virtual ~UnitUIBase();

    virtual bool create(UIContext * ctx) = 0;
    virtual bool destroy(UIContext * ctx);

    virtual DefaultGridUI * gridUI() = 0;
    virtual DefaultUnitUI * unitUI() = 0;
    // virtual BaseWidget * patchUI() = 0;

    void show();
    void hide();

    void updateParameter(slr::ID parameterID, float value);
    const slr::ID id() const;
    const slr::Color & color() const;
    const bool canLoadFiles() const { return _canLoadFiles; }

    const slr::AudioUnitView * view() const { return _view.get(); }
    // slr::AudioUnitView * nonconstview() { return _view.get(); }

    UIContext * uictx() const { return _uictx; }

    protected:
    UIContext * const _uictx;
    bool _canLoadFiles = false;
    const std::shared_ptr<const slr::AudioUnitView> _view;
};


struct UnitControlPopup : public Popup {
    UnitControlPopup(BaseWidget *parent, UIContext * const uictx);
    ~UnitControlPopup();

    UnitUIBase * _currentUnit;
    
    std::unique_ptr<Button> _btnDelete;
    std::unique_ptr<Button> _btnRouteManager;
};

struct DefaultGridUI : public BaseWidget {
    DefaultGridUI(BaseWidget * parent, UnitUIBase *base);
    virtual ~DefaultGridUI();

    void pollFileUpdate();
    virtual void pollUIUpdate();

    // virtual int gridY();
    virtual void setNudge(slr::frame_t nudge, const float horizontalZoom);
    virtual void updatePosition(int x, int y);

    void showFiles();
    void hideFiles();

    std::vector<FileView*> fileList();

    protected:
    UnitUIBase * const _uibase;

    std::unique_ptr<Label> _lblName;
    std::unique_ptr<Label> _lblVolume;
    std::unique_ptr<Button> _btnMute;
    std::unique_ptr<Button> _btnSolo;

    std::vector<std::unique_ptr<FileView>> _fileUIs;
    uint64_t _fileContainerVersion;
    
    bool handleDoubleTap(GestLib::DoubleTapGesture &dt);
};

struct DefaultUnitUI : public BaseWidget {
    DefaultUnitUI(BaseWidget * parent, UnitUIBase *base);
    virtual ~DefaultUnitUI();

    private:
    std::unique_ptr<FileView> _fileViewUI; //for using in grid
    // std::unique_ptr<FileView> _fileEditorUI;
    /* 
        it should be something like... dunno... several buttons as
            - clips - where you can manipulate each clip that is associated with current unit individually
                        (e.g. if there is 3 clips than you select only one of them and can stretch/move/cut/crop/duplicate or whatever)
            
                        but for clips editing it should be somewhat unified interface for all modules. Than, on clip switching - 
                            redraw canvas i guess?

                        so it would be separate things - Clip Edito and File Editor?
                            Editor in general, but there should be than:
                            clips, audio files, midi files, automations, emm... smth else?

                        Plus some simplified timeline

            - modules settings itself - e.g. for Mixer it would be sliders, for OSC it would be some voice controls, adsr, or whatever.
                                        (dunno what to do for track)
                                        
            - something else? mby alternative route mapping menu? where on one side is inputs and on another is outputs

                                Inputs  |        |  Outputs
                                 unit   |        |    unit
                                 unit   |  Unit  |     +
                                  +     |        |
                                        |        |
                maybe :)

            - dunno...
    
    */
};

}