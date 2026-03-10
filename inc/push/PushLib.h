// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <vector>
#include <map>

namespace PushLib {
/* 

https://github.com/Ableton/push-interface/blob/main/doc/AbletonPush2MIDIDisplayInterface.asc#display-interface

based on and inspired by https://github.com/lane-s/libpush 

*/    
constexpr int DISPLAY_HEIGHT = 160;
constexpr int DISPLAY_WIDTH = 960;

constexpr int PEDAL_CURVE_ENTRIES = 32;
constexpr int PAD_VELOCITY_CURVE_ENTRIES = 128;
constexpr int PAD_MATRIX_DIM = 8;
constexpr int TOUCH_STRIP_LEDS = 30;

enum class PushPort { Live, User };

struct LedColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char w;
};

enum class LedAnimationType {
    NoTransition = 0,
    OneShot = 1,
    Pulse = 6,
    Blink = 11
};

enum class LedAnimationDuration {
    d24th = 0,
    sixt = 1,
    eight = 2,
    quater = 3,
    half = 4
};

struct LedAnimation {
    LedAnimationType type;
    LedAnimationDuration duration;
};

using PadNum = unsigned char;
using PadColor = unsigned char;

struct Pad {
    PadNum num;
    PadColor color;
    LedAnimation anim;
};

enum class PadEventType { 
    PadPressed = 0,
    PadReleased,
    PadAftertouch
};

struct PadEvent { 
    PadEventType type;
    int x;
    int y;
    int velocity;
    int padRaw;
};

enum class AftertouchMode {
    Channel = 0,
    Polyphonic
};

enum class PadSensitivity {
    Regular = 0,
    Reduced,
    Low
};

struct PadCalibration {
    int pad[8];
};

enum class Button {
    _DUMMY0 = 0, _DUMMY1 = 1, _DUMMY2 = 2,
    TapTempo        = 3,
    _DUMMY4 = 4, _DUMMY5 = 5, _DUMMY6 = 6, _DUMMY7 = 7, _DUMMY8 = 8,
    Metrnonme       = 9,
    _DUMMY10 = 10, _DUMMY11 = 11, _DUMMY12 = 12, _DUMMY13 = 13, _DUMMY14 = 14, 
    _DUMMY15 = 15, _DUMMY16 = 16, _DUMMY17 = 17, _DUMMY18 = 18, _DUMMY19 = 19,
    DisplayBottom1  = 20,
    DisplayBottom2  = 21,
    DisplayBottom3  = 22,
    DisplayBottom4  = 23,
    DisplayBottom5  = 24,
    DisplayBottom6  = 25,
    DisplayBottom7  = 26,
    DisplayBottom8  = 27,
    Master          = 28,
    StopClip        = 29,
    Setup           = 30,
    Layout          = 31,
    _DUMMY32 = 32, _DUMMY33 = 33, _DUMMY34 = 34,
    Convert         = 35,
    Scene14         = 36,
    Scene14t        = 37,
    Scene18         = 38,
    Scene18t        = 39,
    Scene116        = 40,
    Scene116t       = 41,
    Scene132        = 42,
    Scene132t       = 43,
    Left            = 44,
    Right           = 45,
    Up              = 46,
    Down            = 47,
    Select          = 48,
    Shift           = 49,
    Note            = 50,
    Session         = 51,
    AddDevice       = 52,
    AddTrack        = 53,
    OctaveDown      = 54,
    OctaveUp        = 55,
    Repeat          = 56,
    Accent          = 57,
    Scale           = 58,
    User            = 59,
    Mute            = 60,
    Solo            = 61,
    PageLeft        = 62,
    PageRight       = 63,
    _DUMMY64 = 64, _DUMMY65 = 65, _DUMMY66 = 66, _DUMMY67 = 67, 
    _DUMMY68 = 68, _DUMMY69 = 69, _DUMMY70 = 70, _DUMMY71 = 71, 
    _DUMMY72 = 72, _DUMMY73 = 73, _DUMMY74 = 74, _DUMMY75 = 75, 
    _DUMMY76 = 76, _DUMMY77 = 77, _DUMMY78 = 78, _DUMMY79 = 79, 
    _DUMMY80 = 80, _DUMMY81 = 81, _DUMMY82 = 82, _DUMMY83 = 83, _DUMMY84 = 84, 
    Play            = 85,
    Record          = 86,
    New             = 87,
    Duplicate       = 88,
    Automate        = 89,
    FixedLength     = 90,
    _DUMMY90 = 90, _DUMMY91 = 91, _DUMMY92 = 92, _DUMMY93 = 93, 
    _DUMMY94 = 94, _DUMMY95 = 95, _DUMMY96 = 96, _DUMMY97 = 97, 
    _DUMMY98 = 98, _DUMMY99 = 99, _DUMMY100 = 100, _DUMMY101 = 101,
    DisplayTop1     = 102,
    DisplayTop2     = 103,
    DisplayTop3     = 104,
    DisplayTop4     = 105,
    DisplayTop5     = 106,
    DisplayTop6     = 107,
    DisplayTop7     = 108,
    DisplayTop8     = 109,
    Device          = 110,
    Browser         = 111,
    Mix             = 112,
    Clip            = 113,
    _DUMMY114 = 114, _DUMMY115 = 115,
    Quantize        = 116,
    DoubleLoop      = 117,
    Delete          = 118,
    Undo            = 119,
    _COUNT           = 120
};

enum class ButtonEventType {
    Released = 0,
    Pressed = 1
};

struct ButtonEvent {
    ButtonEventType type;
    Button button;
};

struct ButtonColor {
    Button btn;
    LedAnimation anim;
    unsigned char color;
};

enum class EncoderEventType {
    Touched = 0,
    Moved = 1,
    Released = 2
};

enum class Encoder {
    Tempo       = 14,
    Metronome   = 15,
    Encoder1    = 71,
    Encoder2    = 72,
    Encoder3    = 73,
    Encoder4    = 74,
    Encoder5    = 75,
    Encoder6    = 76,
    Encoder7    = 77,
    Encoder8    = 78,
    Master      = 79
};

struct EncoderEvent {
    EncoderEventType type;
    Encoder encoder;
    int delta;
    int raw;
};

enum class TouchStripEventType {
    Pressed,
    Moved,
    Released
};

struct TouchStripEvent {
    TouchStripEventType type;
    double position;
    int raw;
};

struct TouchStripConfig {
    bool controlledByHost;
    bool ledPoint;
    bool barStartsAtCenter;
    bool autoReturn;
    bool autoReturnToCenter;
};

enum class PedalContact {
    Pedal1Ring = 0,
    Pedal1Tip = 1,
    Pedal2Ring = 2,
    Pedal2Tip = 4
};

struct PedalEvent {
    PedalContact contact;
    double value;
};

struct PedalSampleData {
    unsigned short Pedal1Rign;
    unsigned short Pedal1Tip;
    unsigned short Pedal2Ring;
    unsigned short Pedal2Tip;
};

enum class MidiMode {
    LiveMode = 0,
    UserMode = 1,
    DualMode = 2
};

enum PowerSupplyStatus { 
    USBOnly = 0,
    ExternalSupply = 1
};

struct PushStatus {
    PowerSupplyStatus powerSupplyStatus;
    int uptime;
};

template<typename T>
using ButtonCallbackMap = std::map<PushLib::Button, bool(T::*)(PushLib::ButtonEvent&)>;

template<typename T>
struct ButtonCallback {
    PushLib::Button btn;
    bool (T::*clb)(PushLib::ButtonEvent&);
};

template<typename T>
using BtnCallbackMap = std::array<ButtonCallback<T>, static_cast<int>(Button::_COUNT)>;

}