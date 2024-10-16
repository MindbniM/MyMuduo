#pragma once
#include<unordered_map>
#include"Epoll.hpp"
#include"Channel.hpp"
#include"../fiber/Schedule.hpp"
namespace MindbniM
{
    class EventLoop
    {
    public:
        using ptr=std::shared_ptr<EventLoop>;
        bool insert(int fd,int events,Channel::func_t read=nullptr, Channel::func_t write =nullptr);
        bool erase(int fd);
        bool mod(int fd,int events,Channel::func_t read=nullptr, Channel::func_t write =nullptr);
        void Loop(int timeout=-1);
        void stop();
    private:
        std::atomic<bool> m_run;
        Epoll m_epoll;
        std::unordered_map<int,Channel::ptr> m_Channels;        
    };
    bool EventLoop::insert(int fd,int events,Channel::func_t read, Channel::func_t write)
    {
        if(m_Channels.count(fd)) return false;
        m_epoll.AddFd(fd,events);
        Channel::ptr c=std::make_shared<Channel>(fd,events);
        c->SetReadCall(read);
        c->SetWriteCall(write);
        m_Channels[fd]=c;
        return true;
    }
    bool EventLoop::erase(int fd)
    {
        if(!m_Channels.count(fd))    return false;
        m_epoll.DelFd(fd);
        m_Channels.erase(fd);
        return true;
    }
    bool EventLoop::mod(int fd,int events,Channel::func_t read, Channel::func_t write )
    {
        if(!m_Channels.count(fd)) return false;
        m_epoll.ModFd(fd,events);
        if(read!=nullptr)m_Channels[fd]->SetReadCall(read);
        if(write!=nullptr)m_Channels[fd]->SetWriteCall(write);
        return true;
    }
    void EventLoop::stop()
    {
        m_run=false;
    }
    void EventLoop::Loop(int timeout)
    {
       m_run=true;
        while(m_run)
        {
            int n=m_epoll.Wait(timeout);
            for(int i=0;i<n;i++)
            {
                int fd=m_epoll.GetEventFd(i);
                int event=m_epoll.GetEvents(i);
                if(event&EPOLLHUP) event|=(EPOLLIN|EPOLLOUT);
                if(event&EPOLLERR) event|=(EPOLLIN|EPOLLOUT);
                if(event&EPOLLIN&&m_Channels.count(fd))
                {
                    m_Channels[fd]->_ReadCall();
                }
                if(event&EPOLLOUT&&m_Channels.count(fd))
                {
                    m_Channels[fd]->_WriteCall();
                }
            }
        }
    }
}
