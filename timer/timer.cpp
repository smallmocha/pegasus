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
    printf("tid:%ld, add timer, fd=%d, add=%p\n", syscall(SYS_gettid), timer->id, timer);
    timerList_.insert(timer);
    printf("timerQueue size=%lu\n", timerList_.size());
    return timer;
}

void TimerQueue::Del(Timer *timer)
{
    assert(timer);
    printf("tid:%ld, del timer, fd=%d, remain=%ld, add=%p\n", syscall(SYS_gettid), timer->id, std::chrono::duration_cast<MS>(timer->expires - Clock::now()).count(), timer);
    printf("before timerQueue size=%lu\n", timerList_.size());
    timerList_.erase(timer);
    printf("after timerQueue size=%lu\n", timerList_.size());
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
        printf("tid:%ld, push pires, fd=%d\n", syscall(SYS_gettid), timer->id);
    }
    for (auto &&timer : expiresTimers) {
        printf("fd:%d, add=%p, timeout\n", timer->id, timer);
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
