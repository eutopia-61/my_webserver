#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>
#include <iostream>

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = 8) : pool_(std::make_shared<Pool>())
    {
        assert(threadCount > 0);
        for (size_t i = 0; i < threadCount; ++i)
        {
            printf("Thread pool %ld create\n", i);
            // 匿名函数 [capture](parameters)->return-type{body}
            // unique_lock 实现自动加解锁
            // unique_lock比lock_guard占用空间和速度慢一些，因为其要维护mutex的状态。
            std::thread([pool = pool_]
                        {
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true) {
                    if(!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed) {
                        break;
                    }
                    else{
                        pool->cond.wait(locker);
                    }
                } }).detach();
                
        }
        printf("ThreadPool end!\r\n");
    }

    ThreadPool() = default;

    // 移动构造函数
    ThreadPool(ThreadPool &&) = default;

    ~ThreadPool()
    {
        if (static_cast<bool>(pool_))
        {
            // lock_guard 自动加解锁
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->isClosed = true;
        }
        pool_->cond.notify_all();
    }

    template <typename T>
    void AddTask(T &&task)
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            // forward 完美转发
            pool_->tasks.emplace(std::forward<T>(task));
        }
        pool_->cond.notify_all();
    }

private:
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;

        // std::function实现函数包装器 <返回值类型(参数类型)> 函数指针
        std::queue<std::function<void()>> tasks;
    };

    std::shared_ptr<Pool> pool_;
};