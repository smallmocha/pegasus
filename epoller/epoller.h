#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <vector>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024) : epollFd_(epoll_create1(::EPOLL_CLOEXEC)), events_(maxEvent) 
    {
        assert(epollFd_ > 0 && events_.size() > 0);
    }

    ~Epoller() 
    {
        close(epollFd_);
    }

    bool AddFd(int fd, uint32_t events) 
    {
        epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.data.fd = fd;
        ev.events = events;
        return epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == 0;
    }

    bool ModFd(int fd, uint32_t events) 
    {
        epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.data.fd = fd;
        ev.events = events;
        return epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) == 0;
    }

    bool DelFd(int fd) 
    {
        epoll_event ev;
        bzero(&ev, sizeof(ev));
        return epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev) == 0;
    }

    int Wait(int timeoutMs = -1) 
    {
        return epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    }

    int GetEventFd(size_t i) const 
    {
        assert(i < events_.size());
        return events_[i].data.fd;
    }

    uint32_t GetEvents(size_t i) const 
    {
        assert(i < events_.size());
        return events_[i].events;
    }

private:
    int epollFd_;
    std::vector<epoll_event> events_;    
};

#endif