#pragma once
#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "../log/log.hpp"
namespace MindbniM
{

    // 定时触发的方法
    typedef std::function<void()> TimeoutCall;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::milliseconds MS;
    // 时间戳
    typedef Clock::time_point TimeStamp;

    struct TimerNode
    {
        int id;
        TimeStamp time; // 到期时间
        TimeoutCall cb; // 到期回调方法
        bool operator<(const TimerNode &t)
        {
            return time < t.time;
        }
        bool operator>(const TimerNode &t)
        {
            return time > t.time;
        }
    };
    //小堆
    class Timer
    {
    public:
        Timer() { _heap.reserve(64); }
        ~Timer() { clear(); }
        // 更新一个任务的过期时间
        void updata(int id, int newTime);
        // 如果一个任务存在, 就更新它的到期时间, 如果不存在就添加一个任务
        void add(int id, int timeOut, const TimeoutCall &cb);
        // 删除一个id的任务, 并执行回调
        void doWork(int id);
        // 清空计时器
        void clear();
        // 清理过期任务
        void tick();
        // 删除头元素
        void pop();
        // 获取最近的过期时间
        int GetNextTick();

    private:
        // 删除一个位置
        void del(size_t i);
        // 堆向上调整
        size_t adjust_up(size_t i);
        // 堆向下调整
        size_t adjust_down(size_t index);
        void swap(size_t i, size_t j);

        std::vector<TimerNode> _heap;
        std::unordered_map<int, size_t> _map;
    };
    void Timer::updata(int id, int newTime)
    {
        if(_map.count(id))
        {
            size_t n=_map[id];
            Clock::time_point oldtime=_heap[n].time;
            _heap[n].time=Clock::now()+MS(newTime);
            size_t nn;
            if(oldtime>_heap[n].time) nn=adjust_up(n);
            else nn=adjust_down(n);
            _map[id]=nn;
        }
    }
    void Timer::add(int id, int timeOut, const TimeoutCall &cb)
    {
        //不存在就插入
        if(!_map.count(id))
        {
            size_t n=_heap.size();
            _heap.push_back({id,Clock::now()+MS(timeOut),cb});
            _map[id]=n;
            adjust_up(n);
        }
        else   
        { //存在就更新
            updata(id,timeOut);
            _heap[_map[id]].cb=cb;
        }
    }
    void Timer::doWork(int id)
    {
        if(_map.count(id)) 
        {
            _heap[_map[id]].cb();
            del(_map[id]);
        }
    }
    void Timer::clear()
    {
        _heap.clear();
        _map.clear();
    }
    void Timer::tick()
    {
        while(!_heap.empty())
        {
            if(_heap.front().time<Clock::now())
            {
                _heap.front().cb();
                pop();
            }
            else break;
        }
    }
    void Timer::pop()
    {
        assert(_heap.size()>0);
        del(0);
    }
    int Timer::GetNextTick()
    {
        tick();
        size_t time=0;
        if(!_heap.empty())
        {
            time=std::chrono::duration_cast<MS>(_heap.front().time-Clock::now()).count();
        }
        return time>0? time:0;
    }
    void Timer::del(size_t i)
    {
        assert(i<_heap.size());
        size_t n=_heap.size();
        swap(i,n-1);
        _map.erase(_heap.back().id);
        _heap.pop_back();
        size_t nn=adjust_down(i);
        if(nn==i)
        {
            nn=adjust_up(i);
        }

    }
    size_t Timer::adjust_up(size_t i)
    {
        assert(i<_heap.size());
        int parent=(i-1)/2;
        while(i>0)
        {
            if(_heap[parent]>_heap[i])
            {
                swap(parent,i);
                i=parent;
                parent=(i-1)/2;
            }
            else break;
        }
        return i;
    }
    size_t Timer::adjust_down(size_t index)
    {
        int n=_heap.size();
        size_t child=index*2+1;
        while(child<n)
        {
            if(_heap[child+1]<_heap[child]) child++;
            if(_heap[child]<_heap[index])
            {
                swap(child,index);
                index=child;
                child=index*2+1;
            }
            else break;
        }
        return index;
    }
    void Timer::swap(size_t i, size_t j)
    {
        std::swap(_heap[i],_heap[j]);
        _map[_heap[i].id]=i;
        _map[_heap[j].id]=j;
    }

}