// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "defines.h"
#include "core/primitives/MidiEvent.h"
 
#include "logger.h"

#include <vector>
#include <cmath>
#include <memory>

/* 
        (somewhere in unit)

        float volume = volume; calls

        float ParameterFloat::float() const {
            if(!_isModulating) return _value;

            
            float ret =  0.f;
            
            if(_perBlockAcc) {
                ret = _mod->value; // mod сразу считает единичное значение для блока - как?
                или ret = _value + _mod->values[0];?
            } else {
                ret = _value + _mod->values[frame];
            }
            
            if(ret > _max) ret = _max;
            if(ret < _min) ret = _min;

            return ret;
        }


    тогда modEngine::process(frame, blocksize)  просчитывает значения для текущего смещения и кладет данные в массив.
    Параметр, при вызове value() обращается к своим(в множественном?) модуляциям и берет значение из массива, 
        в зависимости от того необходимо ли sample-accurate или достаточно blocksize?

    можно добавить переключатель между block accurate - тогда кешируем значение lfo в сам параметр и просто на нулевом кадре обновляемся,
      и sample accurate где каждый сэмпл считается значение.
      Или же constant - параметр который меняется только пользователем. ? надо ли такой?


    то есть параметры тогда должны знать о модуляциях, которые их модулируют, или, хотя бы, иметь ссылки на буферы с данными модуляций
    
    тогда надо  события что бы добавить и убрать модуляцию у параметра
*/


namespace slr {

struct AudioContext;

struct ParameterBase;

struct ModulationPattern {
    enum class shape {
        sin,
        saw,
        square,
        triangle,
        custom
    };
    
    ModulationPattern(double sampleRate) : _sampleRate(sampleRate) {
        clearTargets();
    }
    ~ModulationPattern() = default;
    
    frame_t process(const AudioContext &ctx) {
        //fill the _data[frame] with value(frame)
    }

    double value(const frame_t frame) {
        double res = 0.0;

        switch(_shape) {
            case(shape::sin): res = _amplitude * std::sin( 2 * M_PI * (frame /_sampleRate) * _lfoRate + _phase ); break;
            case(shape::saw): res = 0.0; break;
            case(shape::square): res = _amplitude * std::copysign(1.0f, std::sin(2 * M_PI * (frame/_sampleRate) * _lfoRate + _phase) ); break; //https://en.wikipedia.org/wiki/Square_wave_(waveform)
            case(shape::triangle): res = (2*_amplitude / M_PI) * std::asin(std::sin(2 * M_PI *_lfoRate * (frame/_sampleRate) + _phase)); break; //https://en.wikipedia.org/wiki/Triangle_wave
            case(shape::custom): res = 0.0; /* res = data[( ( (frame/_sampleRate or steps?) * _lfoRate) + _phase) % steps] */ break;
        }
        
        // LOG_INFO("frame: %lu, value: %.6f", frame, res);
        return res;
    }

    //must check that unit and parameter exists
    bool addTarget(ID unitID, ID paramID) {
        bool ret = false;
        for(int i=0; i<MAX_TARGETS; ++i) {
            if(_targets[i].unitID == 0) {
                _targets[i].unitID = unitID;
                _targets[i].parameterID = paramID;
                ret = true;
                break;
            }
        }
        return ret;
    }

    bool removeTarget(ID unitID, ID paramID) {
        if(unitID == 0) return false;

        int pos = -1;
        for(int i=0; i<MAX_TARGETS; ++i) {
            if(_targets[i].unitID == unitID && _targets[i].parameterID == paramID) {
                pos = i;
            }
        }

        //failed, no such target
        if(pos == -1) return false;

        for(int i=pos; i<MAX_TARGETS; ++i) {
            if(i+1 != MAX_TARGETS && _targets[i+1].unitID != 0) {
                _targets[i].unitID      = _targets[i+1].unitID;
                _targets[i].parameterID = _targets[i+1].parameterID;
            } else {
                _targets[i].unitID = 0;
                _targets[i].parameterID = 0;
                break;
            }
        }

        return true;
    }

    bool canAddTarget() const { 
        for(int i=0; i<MAX_TARGETS; ++i) {
            if(_targets[i].unitID == 0) return true;
        }
        return false;
    }

    void clearTargets() {
        for(int i=0; i<MAX_TARGETS; ++i) {
            _targets[i].unitID = 0;
            _targets[i].parameterID = 0;
        }
    }

    private:
    double _sampleRate;
    float _amplitude = 1.0f;
    float _phase = 0.0f;
    shape _shape = shape::sin;
    
    float _lfoRate = 1.0f;
    bool _freeRateMode = true; //when true - rate is in hertz(range 0 to ... idk... 1000?), when false - rate represents beat e.g. 1/16. 1/4, etc.

    /* custom shape - screw this? don't need. Better add random with seed
        int steps;
        float * data; 

        std::array<float, steps>
    */

    std::unique_ptr<float> _data;
    struct ModTarget {
        ID unitID;
        ID parameterID;
    };
    
    static const int MAX_TARGETS = 32;
    std::array<ModTarget, MAX_TARGETS> _targets;
};

struct ModulationEngine {
    ModulationEngine();
    ~ModulationEngine();

    frame_t process(const AudioContext &ctx); 
    
    void prepareToPlay();
    void prepareToRecord();
    void stopPlaying();
    void stopRecording();

    private:
    std::vector<std::unique_ptr<ModulationPattern>> _modulations;
};

}