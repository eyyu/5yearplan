/*------------------------------------------------------------------------------
-- SOURCE FILE: timer.h - template headers for timer classes
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
-- Timer(); CTOR
-- ~Timer(); DSTOR
-- start();
-- stop();
-- getRandDuration();
--
-- DATE: NOV 12, 2016
--
-- REVISIONS: 
-- Version 1.0 - [JA] - 2016/NOV/12 - created timer header
-- Version 2.0 - [JA] - 2016/NOV/16 - fixed timer headers to not take class and scoped function pointer 
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- Timer classes here take an unsigned long long representing the number of milliseconds the timer will count for
------------------------------------------------------------------------------*/
#pragma once

#include <thread>
#include <random>
#include <chrono>
#include <condition_variable>

/*--------------------------------------------------------------------------
-- CLASS: Timer<void (*ev)(), unsigned long long minDuration, unsigned long long maxDuration = 100>
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created class
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- This timer class is the random timer and as such contains several private members and 
-- a method not included in the other timer class.
--------------------------------------------------------------------------*/
template<void (*ev)(), unsigned long long minDuration, unsigned long long maxDuration = 100>
class Timer {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool active = false;
    std::chrono::duration<unsigned long long, std::milli> dur;

    std::mt19937_64 randEng{std::random_device()()}; //Predefined 64 bit random engine using random device value as starting seed
    std::uniform_int_distribution<unsigned long long> uni{minDuration, maxDuration}; // guaranteed unbiased

/*--------------------------------------------------------------------------
-- FUNCTION: getRandDuration
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: unsigned long getRandDuration();
--
-- NOTES:
-- This method returns a random duration value guaranteed to be between the min and max durations
--------------------------------------------------------------------------*/
    unsigned long getRandDuration() {
        //return uni(randEng);
        return (randEng() % (maxDuration - minDuration)) + minDuration;
    }

public:

/*--------------------------------------------------------------------------
-- FUNCTION: Timer constructor
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: Timer();
--
-- NOTES:
-- This method sets the duration to a random value at initialization.
--------------------------------------------------------------------------*/
    Timer() : dur(getRandDuration()) {}

/*--------------------------------------------------------------------------
-- FUNCTION: Timer destructor
--
-- DATE: NOV. 19, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: ~Timer();
--
-- NOTES:
-- This method calls the stop method as otherwise, an active timer that gets deleted
-- will leave an active thread orphaned, which results in an error being thrown when it exits.
--------------------------------------------------------------------------*/
    ~Timer() {stop();}

/*--------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void start();
--
-- NOTES:
-- This method utilizes a mutex and a conditional variable to create an
-- interruptable thread. This thread waits for the timers duration, but will be
-- interrupted once the active boolean is set to false. Based on the result of
-- the wait operation, that explains whether the timer ran out vs being stopped
-- prematurely. If the thread has a result of timeout, then the timer callback
-- method is called.
--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------
-- FUNCTION: stop
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created function
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- INTERFACE: void stop();
--
-- NOTES:
-- This method complements the above start method. When called, it sets the
-- active boolean to false and notifies the thread if it is running.
-- It then attempts to join the thread if it is running and joinable.
-- Lastly, it sets the duration to a new random duration.
--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------
-- CLASS: Timer<void(*ev)(), unsigned long duration>
--
-- DATE: DEC. 6, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/19 - created class
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- This timer class is the default timer which utilizes a set duration
-- This class utilizes partial template specialization to "overload" the above timer.
-- Due to the fact that this is essentially a modified copy of the above random timer,
-- I will not be commenting the methods and variables in this class as they are identical to the ones
-- mentioned previously, except for the removal of random elements.
--------------------------------------------------------------------------*/
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