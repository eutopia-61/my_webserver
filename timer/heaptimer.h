#ifndef HEAPTIMER_H_
#define HAEPTIMER_H_

#include <queue>
#include <vector>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock; /*高精度时钟*/
typedef std::chrono::milliseconds MS;             /* 毫秒 */
typedef Clock::time_point TimeStamp;              /* 表示时间点*/

class TimerNode
{
public:
    int id;             /*标记定时器*/
    TimeStamp expires;  /*设置过期时间*/
    TimeoutCallBack cb; /* 回调函数*/

    bool operator<(const TimerNode &t)
    {
        return expires < t.expires;
    }
};

class HeapTimer
{
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    void addTimer(int fd, int timeOut, const TimeoutCallBack &cb); /*设置定时器*/

    void update(int fd, int newExpires);
    void doWork(int id);    /*删除指定id节点，并且触发处理函数*/

    void handle_expired_event(); /*处理过期定时器*/
    int getNextHandle();         /*下一次处理过期定时器*/

    void clear();
    void pop();

private:
    void del_(size_t i);
    void siftup_(size_t i);
    bool siftdown_(size_t index, size_t n);
    void SwapNode_(size_t i, size_t j);

    std::vector<TimerNode> heap_; /*存储定时器*/
    std::unordered_map<int, size_t> ref_;   /*映射定时器在heap_的位置*/
};

#endif