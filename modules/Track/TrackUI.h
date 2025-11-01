// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/UnitUIBase.h"

namespace slr {
    class TrackView;    
    class AudioUnitView;
}

namespace UI {

/* 
Each module need several views:
grid control, gridgrid(for GridView)
module itself -> for last selected module view
patch field(future) -> for patch field view
        (но тут они в 99% случаев будут одинаковые - просто какие нибудь квадратики с названием)
        или ещё что то такое...

mmm what else...
Базовые модули статически линкованы, но регистрируются так же, как плагины.
Ты делаешь общий интерфейс (чистый абстрактный класс IModule или C-подобный struct с функциями).
Для динамических .so у тебя есть extern "C" фабричная функция create_module().
Для встроенных модулей делаешь просто статическую функцию create_builtin_module().
В коде DAW поддерживаешь общий реестр фабрик: часть заполнена через dlopen, часть руками в main() или при инициализации.
Таким образом, разницы при использовании уже нет: у тебя везде объект IModule*.
Примерно:
using ModuleFactory = std::function<IModule*()>;

void register_builtin_modules(Registry& r) {
    r.add("mixer", []{ return new Mixer; });
    r.add("track", []{ return new Track; });
}

void register_dynamic_modules(Registry& r, const std::string& path) {
    void* lib = dlopen(path.c_str(), RTLD_NOW);
    auto create = (IModule*(*)())dlsym(lib, "create_module");
    r.add(path, [create]{ return create(); });
}

*/

class Label;
class Button;
class FileView;
class UIContext;

struct TrackUI : public UnitUIBase {
    TrackUI(slr::AudioUnitView * track, UIContext * uictx);
    ~TrackUI();
    
    bool create(UIContext * ctx) override;
    bool update(UIContext * ctx) override;
    bool destroy(UIContext * ctx) override;

    BaseWidget * gridUI() override { return _gridControl; }
    BaseWidget * moduleUI() override { return _moduleUI; }
    // BaseWidget * patchUI() override;
 
    int gridY() override;
    void setNudge(slr::frame_t nudge, const float horizontalZoom) override;
    void updatePosition(int x, int y) override;

    // std::vector<FileView*> & fileList() { return _viewItems; }
    
    private:
    slr::TrackView * _track;
    // std::vector<FileView*> _viewItems;

    class TrackGridControlUI;
    class TrackModuleUI;
    // class TrackPatchUI;

    TrackGridControlUI * _gridControl;
    TrackModuleUI * _moduleUI;

    struct TrackGridControlUI : public BaseWidget {
        TrackGridControlUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackGridControlUI();

        private:
        TrackUI * _parentUI;

        Label * _lblName; //static
        Label * _lblVolume;
        Button * _btnMute;
        Button * _btnSolo;
        Button * _btnRecord;
        Button * _btnSource;

        bool handleDoubleTap(GestLib::DoubleTapGesture &dt);

        friend class TrackUI;
    };

    struct TrackModuleUI : public BaseWidget {
        TrackModuleUI(BaseWidget *parent, TrackUI * parentUI);
        ~TrackModuleUI();

        private:
        TrackUI * _parentUI;
        std::vector<FileView*> _viewItems;
    
        Label * _name;
        lv_obj_t * _testRect;
        
        friend class TrackUI;
    };

    // struct TrackPatchUI : public BaseWidget {   
    //     TrackPathUI(TrackUI *parent);
    //     ~TrackPathUI();

    //     private:
    //     TrackUI * _parentUI;
    
    //     Label * _name;
    // };
};

}