// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace UI {
    class UnitUIBase;
    class UIContext;
}

namespace slr {

class AudioUnit;
class AudioUnitView;

enum class ModuleType { Basic, BuiltinFX, ExternalFX };
struct Module {
    std::string * _name;
    ModuleType _type;
    //some other parameters??
    std::unique_ptr<slr::AudioUnit> (*createRT)();
    UI::UnitUIBase * (*createUI)(slr::AudioUnitView *, UI::UIContext *);
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