#include "timer.h"
#include <algorithm>
#include <vector>
#include <assert.h>
#include <sys/syscall.h>
#include <unistd.h>

Timer *TimerQueue::Add(int id, int timeout, const TimeoutCallBack &cb)
{
    Timer *timer = new Timer(id, Clock::now() + MS(timeout), cb);
    assert(timer);
    timerList_.insert(timer);
    return timer;
}

void TimerQueue::Del(Timer *timer)
{
    assert(timer);
    timerList_.erase(timer);
    delete timer;
}

void TimerQueue::DelExpiresTimer()
{
    std::vector<Timer *> expiresTimers;
    for(auto &timer : timerList_) {
        if (std::chrono::duration_cast<MS>(timer->expires - Clock::now()).count() > 0) {
            break;
        }
        expiresTimers.push_back(timer);
    }
    for (auto &&timer : expiresTimers) {
        timer->cb();
    }
}

int TimerQueue::GetNextTick()
{
    int nextTick = -1;
    DelExpiresTimer();
    if (!timerList_.empty()) {
        nextTick = std::chrono::duration_cast<MS>((*timerList_.begin())->expires - Clock::now()).count();
        if (nextTick < 0) {
            nextTick = 0;
        }
    }
    return nextTick;
}
