// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "defines.h"
#include "core/primitives/MidiEvent.h"
#include "common/ModEngineCommon.h"
#include "common/Color.h"

#include <vector>
#include <memory>
#include <atomic>

namespace slr {
struct ModulationPattern;

struct ModulationPatternView {
    ModulationPatternView(const ModulationPattern* ptrn);
    ~ModulationPatternView();

    void update();

    ModulationShape shape() const;
    float rate() const { return _rate; }
    bool rateMode() const { return _rateMode; }
    float phase() const { return _phase; }
    float amplitude() const { return _amplitude; }
    float min() const { return _min; }
    float max() const { return _max; }

    ModTargetArray targets() const { return _targets; }

    ID id() const;
    const Color & color() const { return _uniqueColor; }
    void color(Color clr) { _uniqueColor = clr; incrementVersion(); }

    uint64_t version() const { return _version.load(std::memory_order_acquire); }

    private:
    const ModulationPattern * _ptrn;

    ModulationShape _shape = ModulationShape::sin;
    float _amplitude = 1.0f;
    float _phase = 0.0f;
    float _rate = 1.0f;
    bool _rateMode;
    float _min = -1.0f;
    float _max = 1.0f;

    ModTargetArray _targets;
    Color _uniqueColor;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
};

struct ModulationEngineView {
    ModulationEngineView();
    ~ModulationEngineView() = default;

    std::shared_ptr<ModulationPatternView> createModulationView(const ModulationPattern *ptrn);
    std::shared_ptr<ModulationPatternView> findPatternById(ID id);


    private:
    std::vector<std::shared_ptr<ModulationPatternView>> _storage;
};

}