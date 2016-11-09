#pragma once

#include <thread>
#include <random>

/*
Generic timer code goes in here
Since we all need timers, its better for us to have a single timer class that everyone uses
instead of 4 different classes that quadruple the chances of bugs in our implementation

Timers classes here take an unsigned long representing the number of milliseconds the timer will count for
*/

//Default timer with member function pointer
template<typename T, void (T::*ev)(), unsigned long duration>
class Timer {
    std::thread thr;

public:
    Timer();
    void start();
    void stop();
    void reset();

};

//Random timer with member function pointer
template<typename T, void(*ev)(), unsigned long minDuration = 0, unsigned long maxDuration = 100>
class Timer {
    //Predefined 64 bit random engine
    std::mt19937_64 rand;

    std::thread thr;

public:
    Timer();
    void start();
    void stop();
    void reset();

};