#pragma once

#include <cmath>
#include <limits>

namespace sMath {

/* 
    from 
    https://stackoverflow.com/questions/17333/how-do-you-compare-float-and-double-while-accounting-for-precision-loss
    
    good to read as well https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    ? https://lisyarus.github.io/blog/posts/its-ok-to-compare-floating-points-for-equality.html
*/

// Source - https://stackoverflow.com/a/41405501
// Posted by Shital Shah, modified by community. See post 'Timeline' for change history
// Retrieved 2026-08-21, License - CC BY-SA 4.0

//implements relative method - do not use for comparing with zero
//use this most of the time, tolerance needs to be meaningful in your context
template<typename TReal>
inline bool isApproximatelyEqual(TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon())
{
    TReal diff = std::fabs(a - b);
    if (diff <= tolerance)
        return true;

    if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance)
        return true;

    return false;
}

//supply tolerance that is meaningful in your context
//for example, default tolerance may not work if you are comparing double with float
template<typename TReal>
inline bool isApproximatelyZero(TReal a, TReal tolerance = std::numeric_limits<TReal>::epsilon())
{
    if (std::fabs(a) <= tolerance)
        return true;
    return false;
}


//use this when you want to be on safe side
//for example, don't start rover unless signal is above 1
template<typename TReal>
inline bool isDefinitelyLessThan(TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon())
{
    TReal diff = a - b;
    if (diff < tolerance)
        return true;

    if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance)
        return true;

    return false;
}

template<typename TReal>
inline bool isDefinitelyGreaterThan(TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon())
{
    TReal diff = a - b;
    if (diff > tolerance)
        return true;

    if (diff > std::fmax(std::fabs(a), std::fabs(b)) * tolerance)
        return true;

    return false;
}

//implements ULP method
//use this when you are only concerned about floating point precision issue
//for example, if you want to see if a is 1.0 by checking if its within
//10 closest representable floating point numbers around 1.0.
template<typename TReal>
inline bool isWithinPrecisionInterval(TReal a, TReal b, unsigned int interval_size = 1)
{
    TReal min_a = a - (a - std::nextafter(a, std::numeric_limits<TReal>::lowest())) * interval_size;
    TReal max_a = a + (std::nextafter(a, std::numeric_limits<TReal>::max()) - a) * interval_size;

    return min_a <= b && max_a >= b;
}

constexpr float TWOPI = 2.0*M_PI;
constexpr float TWOPIF = 2.0*M_PIf;
constexpr double TWOPID = 2.0*M_PIf;

template<typename T>
inline T abs(T val) {
    return std::abs(val);
}

template<typename T>
inline T round(T val) {
    return std::round(val);
}

template<typename T>
inline T clamp(T val, T min, T max) {
    // return std::max(std::min(val, max), min); ?
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

template<typename T>
inline T lerp(T val, T minFrom, T maxFrom, T minTo, T maxTo) {
    return (val - minFrom) * (maxTo - minTo) / (maxFrom - minFrom) + minTo;
}

template<typename T>
inline T floor(T val) {
    return std::floor(val);
}

inline double sin(double val) {
    return std::sin(val);
}

inline double asin(double val) {
    return std::asin(val);
}

inline double tan(double val) {
    return std::tan(val);
}

inline double atan(double val) {
    return std::atan(val);
}


/*  frame <= sampleRate
    0.0 <= phase < 1.0 
    return amplitude * sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF)); */
template<typename T>
inline T sineWave(const T &amplitude, const T &frame, const T &sampleRate, const T &rate, const T &phase) {
    return amplitude * sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF));
}

/*  frame <= sampleRate
    0.0 <= phase < 1.0 
    T tmp = ((frame*rate)/sampleRate) + phase; 
    return 2.0 * (tmp - ( 0.5 + sMath::floor(tmp) ) ); */
template<typename T>
inline T sawtoothWave(const T &amplitude, const T &frame, const T &sampleRate, const T &rate, const T &phase) {
    T tmp = ((frame*rate)/sampleRate) + phase; //https://en.wikipedia.org/wiki/Sawtooth_wave
    return 2.0 * (tmp - ( 0.5 + sMath::floor(tmp) ) );

}

/*  frame <= sampleRate
    0.0 <= phase < 1.0 
    return amplitude * std::copysign(1.0f, sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF)) ); */
template<typename T>
inline T squareWave(const T &amplitude, const T &frame, const T &sampleRate, const T &rate, const T &phase) {
    return amplitude * std::copysign(1.0f, sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF)) ); //https://en.wikipedia.org/wiki/Square_wave_(waveform)
} 

/*  frame <= sampleRate
    0.0 <= phase < 1.0 
    return (2*amplitude / M_PI) * sMath::asin(sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF))); */
template<typename T>
inline T triangleWave(const T &amplitude, const T &frame, const T &sampleRate, const T &rate, const T &phase) {
    return (2*amplitude / M_PI) * sMath::asin(sMath::sin(TWOPIF * (frame/sampleRate) * rate + (phase*TWOPIF))); //https://en.wikipedia.org/wiki/Triangle_wave
}

} //sMath
