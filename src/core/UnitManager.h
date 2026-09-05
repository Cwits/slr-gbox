// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ClipContainer.h"
#include "common/defines.h"

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

enum class UnitType { Basic, BuiltinFX, ExternalFX };

struct UnitDescriptor {
    const std::string_view * _name;
    // ModuleType _type;
    //some other parameters??
    std::unique_ptr<slr::AudioUnit> (*createRT)(const ClipContainer *, const ID forcedId);
    std::shared_ptr<slr::AudioUnitView> (*createView)(slr::AudioUnit *);
    std::unique_ptr<UI::UnitUIBase> (*createUI)(const std::shared_ptr<const slr::AudioUnitView> &, UI::UIContext *);
    std::unique_ptr<PushUI::UnitUIBase> (*createPushUI)(const std::shared_ptr<const slr::AudioUnitView> &, PushUI::PushUIContext *); 
};

struct UnitManagerFactory {
    static void init();
    static void registerUnit(const UnitDescriptor* module) {
        inst()._unitDescriptorList.push_back(module);
    }
    static void discoverUnits();
    static const UnitDescriptor * findUnit(std::string name);

    private:
    static UnitManagerFactory & inst() {
        static UnitManagerFactory unitManager;
        return unitManager;
    }
    std::vector<const UnitDescriptor *> _unitDescriptorList;

};

} //namespace slr