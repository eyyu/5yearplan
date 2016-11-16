#pragma once

#include <thread>
#include <random>
#include <chrono>

/*
Timer classes here take an unsigned long representing the number of milliseconds the timer will count for
*/

template<typname T, void (T::*ev)()>
class TimerBase {
protected:
    std::thread thr;
    //Used in sleep_for in C++11
    std::chrono::duration<unsigned long, std::milli> dur;
    bool active = false;
public:
    Timer() = 0;
    void start() = 0;
    void stop() = 0;
    void reset() = 0;
};

//Default timer with member function pointer
template<typename T, void (T::*ev)(), unsigned long duration>
class Timer : TimerBase<T, T::*ev> {
public:
    Timer() : dur(duration) {}

    void stop() {
        if (!active) {
            return;
        }
    }

    void reset() {
        stop();
        active = false;
    }
};

//Random timer with member function pointer
template<typename T, void(T::*ev)(), unsigned long minDuration, unsigned long maxDuration>
class Timer : TimerBase<T, T::*ev> {
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937_64 randEng{rd()}; //Predefined 64 bit random engine
    std::uniform_int_distribution<long> uni{minDuration, maxDuration}; // guaranteed unbiased

    unsigned long getRandDuration() {
        return std::abs(uni(randEng));
    }

public:
    Timer() : dur(getRandDuration()) {}

    void stop() {
        if (!active) {
            return;
        }
    }

    void reset() {
        stop();
        active = false;
        dur = getRandDuration();
    }
};