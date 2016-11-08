#pragma once

#include <functional>
#include <thread>

/*
Generic timer code goes in here
Since we all need timers, its better for us to have a single timer class that everyone uses
instead of 4 different classes that quadruple the chances of bugs in our implementation

Timers classes here take an unsigned long representing the number of milliseconds the timer will count for
*/

template<std::function event, unsigned long duration>
class Timer {

};

template<std::function event, unsigned long minDuration = 0, unsigned long maxDuration = 1000>
class Timer {

};