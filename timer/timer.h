#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <set>
#include <functional>

using Clock = std::chrono::high_resolution_clock;
using TimeStamp = Clock::time_point;
using MS = std::chrono::milliseconds;
using TimeoutCallBack = std::function<void()>;

struct Timer {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;

    Timer(int id, TimeStamp expires, TimeoutCallBack cb) {
        this->id = id;
        this->expires = expires;
        this->cb = cb;
    }
};

struct TimerCmp {
    bool operator()(Timer *a, Timer *b) {
        if (a->expires < b->expires) {
            return true;
        }
        return a->id < b->id;
    }
};

class TimerQueue {
public:
    TimerQueue() = default;
    ~TimerQueue() = default;

    Timer *Add(int id, int timeout, const TimeoutCallBack &cb);
    void Del(Timer *timer);
    int GetNextTick();

private:
    void DelExpiresTimer();

private:
    std::set<Timer *, TimerCmp> timerList_;
};

#endif