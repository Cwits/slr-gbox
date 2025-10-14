// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once 

#include "snapshots/ParameterView.h"
#include "snapshots/ParameterArrayView.h"
#include "snapshots/FileContainerView.h"
#include "defines.h"
#include "Color.h"

namespace slr {

class AudioUnit;

class AudioUnitView {
    public:
    AudioUnitView(AudioUnit * au);
    virtual ~AudioUnitView();
    
    void addParameter(ParameterBaseView * base);
    void setParameter(ID parameterId, float value) { _flatParameterList.operator[](parameterId)->setValue(value); }
    
    virtual void update();

    const std::string & name() const { return _name; }
    void setName(const std::string &name) { _name = name; }

    const bool mute() const { return static_cast<float>(*_mute); }
    void setMute(bool mute) { *_mute = mute; }
    const ID muteId() const { return _mute->id(); }
    const float volume() const { return static_cast<float>(*_volume); }
    void setVolume(float volume) { *_volume = volume; }
    const ID volumeId() const { return _volume->id(); }
    const float pan() const { return static_cast<float>(*_pan); }
    void setPan(float pan) { *_pan = pan; }
    const ID panId() const { return _pan->id(); }
    
    const ID id() const { return _uniqueId; }
    
    const Color & color() const { return _uniqueColor; }

    FileContainerView _fileList;

    protected:
    const AudioUnit * _au;
    const ID _uniqueId;
    bool _solo;
    
    Color _uniqueColor;

    ParameterFloatView * _volume;
    ParameterFloatView * _pan;
    ParameterBoolView * _mute;
    ParameterArrayView _flatParameterList;
    
    std::string _name;

};
    

}