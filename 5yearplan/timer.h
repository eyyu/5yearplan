#pragma once

#include <thread>
#include <random>
#include <chrono>
#include <condition_variable>

/*
Timer classes here take an unsigned long representing the number of milliseconds the timer will count for
*/

//Abstract virtual base class
template<typname T, void (T::*ev)()>
class TimerBase {
protected:
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool active = false;
    std::chrono::duration<unsigned long, std::milli> dur;
public:
    virtual Timer() = 0;
    void start() {
        stop();
        {
            auto lock = std::unique_lock<auto>(mutex);
            active = true;
        }
        thread = std::thread([&] {
            auto lock = std::unique_lock<auto>(mutex);
            while (active) {
                auto result = cv.wait_for(lock, dur);
                if (result == std::cv_status::timeout) {
                    ev();
                }
            }
        });
    }
    virtual void stop() {
        {
            auto lock = std::unique_lock<auto>(mutex);
            active = false;
        }
        cv.notify_one();
        if (thread.joinable) {
            thread.join();
        }
    }
};

//Default timer
template<typename T, void (T::*ev)(), unsigned long duration>
class Timer : TimerBase<T, T::*ev> {
public:
    Timer() : dur(duration) {}
};

//Random timer
template<typename T, void(T::*ev)(), unsigned long minDuration, unsigned long maxDuration>
class Timer : TimerBase<T, T::*ev> {
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937_64 randEng{rd()}; //Predefined 64 bit random engine
    std::uniform_int_distribution<unsigned long> uni{minDuration, maxDuration}; // guaranteed unbiased

    unsigned long getRandDuration() {
        return uni(randEng);
    }

public:
    Timer() : dur(getRandDuration()) {}
    void stop() {
        TimerBase<T, T::*ev>::stop();
        dur = getRandDuration();
    }
};