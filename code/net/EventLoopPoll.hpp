#pragma once
#include "EventLoop.hpp"
#include"../fiber/Schedule.hpp"
namespace MindbniM
{
    class EventLoopPoll
    {
    public:
        EventLoopPoll(const Socket& listen,int PollNum=3);
    private:
        EventLoop::ptr m_root;
        std::vector<EventLoop::ptr> m_loops;
    };
}
