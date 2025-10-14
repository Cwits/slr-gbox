// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/SettingsManager.h"
#include "snapshots/TimelineView.h"
#include "defines.h"

#include <string>
#include <cctype>
#include <cmath>
#include <vector>
#include <algorithm>

namespace UIUtility {

inline std::string bpmToString(const float &bpm) {
    std::string bpms = std::to_string(bpm);
    bpms = bpms.substr(0, bpms.find_first_of('.')+3);
    // bpms.append(" bpm");
    return bpms;
}

inline std::string signatureToString(const slr::BarSize &sig) {
    std::string sigs;
    sigs = std::to_string(sig._numerator);
    sigs.append("/");
    sigs.append(std::to_string(sig._denominator));
    return sigs;
}

inline float stringToBpm(const std::string &text) {
    auto check = [](const std::string &t) -> bool {
        if(t.empty()) return false;

        std::size_t dotPos = t.find_first_of('.');
        if(dotPos == std::string::npos) return false;

        std::string whole = t.substr(0, dotPos);
        std::string fraction = t.substr(dotPos+1, std::string::npos);

        if(!std::all_of(whole.begin(), whole.end(), ::isdigit)) return false;
        if(!std::all_of(fraction.begin(), fraction.end(), ::isdigit)) return false;

        return true;
    };

    if(!check(text)) {
        return -1.0f;
    }
    
    return std::round(std::stod(text) * 100.0f) / 100.0f;
}

inline slr::BarSize stringToSig(const std::string &text) {
    auto check = [](const std::string &t) -> bool {
        if(t.empty()) return false;

        std::size_t divider = t.find_first_of('/');
        if(divider == std::string::npos) return false;

        std::string before = t.substr(0, divider);
        std::string after = t.substr(divider+1, std::string::npos);

        if(!std::all_of(before.begin(), before.end(), ::isdigit)) return false;
        if(!std::all_of(after.begin(), after.end(), ::isdigit)) return false;

        return true;
    };

    if(!check(text)) {
        return slr::BarSize{0, 0};
    }

    std::size_t divider = text.find_first_of('/');
    std::string before = text.substr(0, divider);
    std::string after = text.substr(divider+1, std::string::npos);

    uint8_t num = static_cast<uint8_t>(std::stoi(before));
    uint8_t den = static_cast<uint8_t>(std::stoi(after));

    return slr::BarSize{num, den};
}

inline slr::frame_t textToLoop(const std::string &text, slr::frame_t bar, slr::frame_t quater, bool &failed) {
    if(std::all_of(text.begin(), text.end(), ::isdigit)) {
        return std::stoi(text);
    } 

    //not all are digits...
    //operations * and +
    
    //allowed tokens: bar, quater, +, *
    auto str_tolower = [](std::string s) -> std::string {
        std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c){ return std::tolower(c); } // correct
        );
        return s;
    };

    std::string toParse = str_tolower(text);
    
    //thanks to chatgpt
    struct Token {
        enum Type { Number, Plus, Mul } type;
        long long value;
    };

    auto evaluate = [](const std::string& expr, slr::frame_t bar, slr::frame_t quater, bool &failed) -> slr::frame_t {
        std::vector<Token> tokens;
        for (size_t i = 0; i < expr.size();) {
            if (std::isspace((unsigned char)expr[i])) {
                ++i;
            } else if (std::isdigit((unsigned char)expr[i])) {
                long long v = 0;
                while (i < expr.size() && std::isdigit((unsigned char)expr[i])) {
                    v = v * 10 + (expr[i] - '0');
                    ++i;
                }
                tokens.push_back({Token::Number, v});
            } else if (std::isalpha((unsigned char)expr[i])) {
                std::string id;
                while (i < expr.size() && std::isalpha((unsigned char)expr[i])) {
                    id.push_back(expr[i]);
                    ++i;
                }
                long long v = 0;
                if (id == "bar") v = bar;
                else if (id == "quater") v = quater;
                // else throw std::runtime_error("Unknown identifier: " + id);
                else failed = true;

                tokens.push_back({Token::Number, v});
            } else if (expr[i] == '+') {
                tokens.push_back({Token::Plus, 0});
                ++i;
            } else if (expr[i] == '*') {
                tokens.push_back({Token::Mul, 0});
                ++i;
            } else {
                // throw std::runtime_error("Unexpected character");
                failed = true;
            }
        }

        std::vector<Token> reduced;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].type == Token::Mul) {
                long long lhs = reduced.back().value;
                long long rhs = tokens[i+1].value;
                reduced.back().value = lhs * rhs;
                ++i; 
            } else {
                reduced.push_back(tokens[i]);
            }
        }

        long long result = 0;
        for (auto& t : reduced) {
            if (t.type == Token::Number) result += t.value;
        }
        return result;
    };

    return evaluate(toParse, bar, quater, failed);
}
/*
inline int pixelPerBar(float horizontalZoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    uint8_t beatsPerBar = tl.getBarSize()._numerator;
    uint8_t beatUnit = tl.getBarSize()._denominator;
    float bpm = tl.bpm();

    float barsPerMinute = bpm / beatUnit / beatsPerBar;
    return std::round(slr::SettingsManager::getDefaultPixelPerBar() * horizontalZoom / barsPerMinute);

    //44100
    //float quaterMul = 4 / tl.denominator; //4 = 1, 8 = 0.5, 2 = 2, 1 = 4
    //float tickLength = 60 / 120 = 0.5s
    //float timePerTick = tickLength * quaterMul; //0.5 * 1 = 0.5
    //float timePerBar = (tl.numerator * timePerTick); // 0.5 * 4 = 2
    //float smth = tl.framesPerBar / timePerBar;  // 88200 / 2 = 44100
    //float res = smth / sampleRate;
    //float pixelPerBar = DEFAULT * (horizontalZoom / res); 

    //float framesPerPixel = tl.framesPerBar / DEFAULT
    //framesPerPixel = defalut;(588)
    //return framesPerBar / framesPerPixel
    //

}*/
inline float pixelPerBar(float horizontalZoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    return (tl.framesPerBar() / slr::SettingsManager::getFramesPerPixel()) * horizontalZoom;
}

inline float framesPerPixel(float horizontalZoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    int pixPerBar = UIUtility::pixelPerBar(horizontalZoom);
    float framesPerPixel = (float)pixPerBar / tl.framesPerBar();
    return framesPerPixel;
}

inline float frameToPixel(slr::frame_t frame, float horizontalZoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    int pixPerBar = pixelPerBar(horizontalZoom);
    float notFramesPerPixel = (float)pixPerBar / tl.framesPerBar();
    return std::round(notFramesPerPixel*frame);
}

inline slr::frame_t pixelToFrame(int pixel, float horizontalZoom) {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    int pixPerBar = pixelPerBar(horizontalZoom);
    float notFramesPerPixel = (float)pixPerBar / tl.framesPerBar();
    return pixel/notFramesPerPixel;
}

}