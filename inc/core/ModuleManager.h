// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ClipContainer.h"
#include <string>
#include <vector>
#include <memory>

namespace UI {
    class UnitUIBase;
    class UIContext;
}

namespace PushUI {
    class UnitUIBase;
    class PushUIContext;
}

namespace slr {

class AudioUnit;
class AudioUnitView;

enum class ModuleType { Basic, BuiltinFX, ExternalFX };
struct Module {
    const std::string_view * _name;
    ModuleType _type;
    //some other parameters??
    std::unique_ptr<slr::AudioUnit> (*createRT)(const ClipContainer *);
    std::unique_ptr<slr::AudioUnitView> (*createView)(slr::AudioUnit *);
    std::unique_ptr<UI::UnitUIBase> (*createUI)(slr::AudioUnitView *, UI::UIContext *);
    std::unique_ptr<PushUI::UnitUIBase> (*createPushUI)(slr::AudioUnitView *, PushUI::PushUIContext *); 
};

struct ModuleManagerFactory {
    static void init();
    static void registerModule(const Module * module) {
        inst()._moduleList.push_back(module);
    }
    static void discoverModules();
    static const Module * findModule(std::string name);

    private:
    static ModuleManagerFactory & inst() {
        static ModuleManagerFactory moduleManager;
        return moduleManager;
    }
    std::vector<const Module *> _moduleList;

};

} //namespace slr