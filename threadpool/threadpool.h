#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>
#include <sys/syscall.h>
#include <unistd.h>

class ThreadPool : public std::enable_shared_from_this<ThreadPool>
{
public:
    ThreadPool() : isClosed_(false) {}
    
    ~ThreadPool() 
    {
        {
            std::lock_guard<std::mutex> locker(mutex_);
            isClosed_ = true;
        }
        cond_.notify_all();
    }

    void Init(int threadNum) 
    {
        for (int i = 0; i < threadNum; ++i) {
            std::thread([pool = shared_from_this()](){
                std::unique_lock<std::mutex> locker(pool->mutex_);
                while(true) {
                    if (!pool->taskQue_.empty()) {
                        auto task = std::move(pool->taskQue_.front());
                        pool->taskQue_.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    } else if (pool->isClosed_) {
                        break;
                    } else {
                        pool->cond_.wait(locker);
                    }
                }
            }).detach();
        }
    }

    template<class F>
    void AddTask(F &&task) 
    {
        {
            std::lock_guard<std::mutex> locker(mutex_);
            taskQue_.push(std::forward<F>(task));
        }
        cond_.notify_one();
    }

private:
    bool isClosed_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<std::function<void()>> taskQue_;
};

#endif