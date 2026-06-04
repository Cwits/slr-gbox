// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once 

#include "snapshots/ParameterView.h"
#include "snapshots/ParameterArrayView.h"
#include "snapshots/FileContainerView.h"
#include "defines.h"
#include "common/Color.h"

#include <vector>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <nlohmann/json.hpp>

namespace slr {

class AudioUnit;

class AudioUnitView {
    public:
    AudioUnitView(AudioUnit * au);
    virtual ~AudioUnitView();

    void setParameter(ID parameterId, float value);
    ParameterArrayView & allParameters() { return _flatParameterList; }

    virtual void update() {}

    const std::string & name() const { return _name; }
    void setName(const std::string &name) { _name = name; incrementVersion(); }

    const bool mute() const { return *_mute; }
    const ID muteId() const { return _mute->id(); }
    const float volume() const { return *_volume; }
    const ID volumeId() const { return _volume->id(); }
    const float pan() const { return *_pan; }
    const ID panId() const { return _pan->id(); }

    // void setMute(bool mute);
    // void setVolume(float volume);
    // void setPan(float pan);
    
    const ID id() const { return _uniqueId; }
    
    const Color & color() const { return _uniqueColor; }
    void color(Color clr) { _uniqueColor = clr; incrementVersion(); }

    const bool isMidiThru() const { return _midiThru; }
    const bool isOmniHwInput() const { return _omniHwInput; }

    void setMidiThru(bool state) { _midiThru = state; incrementVersion(); }
    void setOmniHw(bool state) { _omniHwInput = state; incrementVersion(); }

    void appendClipItem(ClipItemView * item);

    ClipContainerView _clipContainer;

    uint64_t version() const;

    //for serialization
    
    /*
        override for each of units.
        for external(e.g. lv2) it should be 
        external:uri

        for internal:
        internal:type - e.g. internal:track or internal:mixer
    */
    virtual std::string unitType() { return "null"; } 

    //override this two if need to save/load unit
    virtual nlohmann::ordered_json saveUnit();
    virtual void loadUnit(const nlohmann::json &data);

    protected:
    const AudioUnit * _au;
    const ID _uniqueId;

    
    std::atomic<uint64_t> _version;
    void incrementVersion();
    
    bool _solo;
    
    Color _uniqueColor;
    
    void addParameter(ParameterBaseView * base);
    ParameterFloatView * _volume;
    ParameterFloatView * _pan;
    ParameterBoolView * _mute;
    ParameterArrayView _flatParameterList;
    
    std::string _name;

    bool _midiThru;
    bool _omniHwInput;

};
    

}