#pragma once

#include <thread>
#include <random>
#include <chrono>
#include <condition_variable>

/*
Timer classes here take an unsigned long representing the number of milliseconds the timer will count for
*/

//Random timer
template<void (*ev)(), unsigned long long minDuration, unsigned long long maxDuration = 100>
class Timer {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool active = false;
    std::chrono::duration<unsigned long long, std::milli> dur;

    std::mt19937_64 randEng{std::random_device()()}; //Predefined 64 bit random engine using random device value as starting seed
    std::uniform_int_distribution<unsigned long long> uni{minDuration, maxDuration}; // guaranteed unbiased

    unsigned long getRandDuration() {
        //return uni(randEng);
		return (randEng() % (maxDuration - minDuration)) + minDuration;
    }

public:
    Timer() : dur(getRandDuration()) {}
    ~Timer() {stop();}
    void start() {
        stop();
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            active = true;
        }
        thread = std::thread([&] {
            auto lock = std::unique_lock<std::mutex>(mutex);
            while (active) {
                auto result = cv.wait_for(lock, dur);
                if (result == std::cv_status::timeout) {
                    ev();
                }
            }
        });
    }
    void stop() {
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            active = false;
        }
        cv.notify_one();
        if (thread.joinable()) {
            thread.join();
        }
        dur = std::chrono::duration<unsigned long, std::milli>(getRandDuration());
    }
};

//Default timer
template<void(*ev)(), unsigned long duration>
class Timer<ev, duration, 100> {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool active = false;
    std::chrono::duration<unsigned long, std::milli> dur;

public:
    Timer() : dur(duration) {}
    ~Timer() {stop();}
    void start() {
        stop();
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            active = true;
        }
        thread = std::thread([&] {
            auto lock = std::unique_lock<std::mutex>(mutex);
            while (active) {
                auto result = cv.wait_for(lock, dur);
                if (result == std::cv_status::timeout) {
                    ev();
                }
            }
        });
    }
    void stop() {
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            active = false;
        }
        cv.notify_one();
        if (thread.joinable()) {
            thread.join();
        }
    }
};