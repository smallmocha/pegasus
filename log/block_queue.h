#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <assert.h>

template<typename T>
class BlockQueue {
public:
    explicit BlockQueue(size_t capacity = 1000) : capacity_(capacity),  isClose_(false)
    {
        assert(capacity > 0);
    }

    ~BlockQueue()
    {
        Close();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> locker(mtx_);
        return que_.empty();
    }

    bool Full()
    {
        std::lock_guard<std::mutex> locker(mtx_);
        return que_.size() >= capacity_;
    }

    size_t Size()
    {
        std::lock_guard<std::mutex> locker(mtx_);
        return que_.size();
    }

    size_t Capacity()
    {
        std::lock_guard<std::mutex> locker(mtx_);
        return capacity_;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> locker(mtx_);
        que_.clear();
    }

    void Close()
    {
        {
            std::lock_guard<std::mutex> locker(mtx_);
            que_.clear();
            isClose_ = true;
        }
        condProducer_.notify_all();
        condComsumer_.notify_all();
    }

    void PushBack(const T &item);
    void PushFront(const T&item);

    bool Pop(T &item);
    bool Pop(T &item, int timeout);

    void Flush()
    {
        condComsumer_.notify_one();
    }

private:
    size_t capacity_;
    bool isClose_;
    std::mutex mtx_;
    std::deque<T> que_;
    std::condition_variable condProducer_;
    std::condition_variable condComsumer_;
};

template<typename T>
void BlockQueue<T>::PushBack(const T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (que_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    que_.push_back(item);
    condComsumer_.notify_one();
}

template<typename T>
void BlockQueue<T>::PushFront(const T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (que_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    que_.push_front(item);
    condComsumer_.notify_one();
}

template<typename T>
bool BlockQueue<T>::Pop(T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (que_.empty()) {
        condComsumer_.wait(locker);
        if (isClose_) {
            return false;
        }
    }
    item = que_.front();
    que_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<typename T>
bool BlockQueue<T>::Pop(T &item, int timeout)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (que_.empty()) {
        if (condComsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
            return false;
        }
        if (isClose_) {
            return false;
        }
    }
    item = que_.front();
    que_.pop_front();
    condProducer_.notify_one();
    return true;
}

#endif // !BLOCK_QUEUE_H
