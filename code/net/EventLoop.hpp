#pragma once
#include<unordered_map>
#include"Epoll.hpp"
#include"Channel.hpp"
#include"TcpConnect.hpp"
#include"../fiber/Schedule.hpp"
namespace MindbniM
{
    
    class EventLoop
    {
    public:
        using ptr=std::shared_ptr<EventLoop>;
        bool insert(Channel::ptr c);
        bool erase(int fd);
        bool mod(int fd,int events,Channel::func_t read=nullptr, Channel::func_t write =nullptr);
        void Loop(int timeout=-1);
        void stop();
    private:
        std::atomic<bool> m_run;
        Epoll m_epoll;
        std::unordered_map<int,Channel::ptr> m_Channels;        
        std::mutex m_mutex;
    };
    bool EventLoop::insert(Channel::ptr c)
    {
        if(m_Channels.count(c->Fd())) return false;
        m_epoll.AddFd(c->Fd(),c->Events());
        std::lock_guard<std::mutex> g(m_mutex);
        m_Channels[c->Fd()]=c;
        std::string ev;
        if(c->Events()&EPOLLET) ev+=" EPOLLET ";
        if(c->Events()&EPOLLIN) ev+=" EPOLLIN ";
        if(c->Events()&EPOLLOUT) ev+=" EPOLLOUT ";
        std::cout<<std::static_pointer_cast<TcpConnect>(c)->Fd();
        LOG_ROOT_DEBUG<<"fd: "<<c->Fd()<<" 已添加 Events: "<<ev;
        return true;
    }
    bool EventLoop::erase(int fd)
    {
        if(!m_Channels.count(fd))    return false;
        m_epoll.DelFd(fd);
        std::lock_guard<std::mutex> g(m_mutex);
        m_Channels.erase(fd);
        LOG_ROOT_DEBUG<<"fd: "<<fd<<" 已删除";
        return true;
    }
    bool EventLoop::mod(int fd,int events,Channel::func_t read, Channel::func_t write )
    {
        if(!m_Channels.count(fd)) return false;
        m_epoll.ModFd(fd,events);
        if(read!=nullptr)m_Channels[fd]->SetReadCall(read);
        if(write!=nullptr)m_Channels[fd]->SetWriteCall(write);
        std::lock_guard<std::mutex> g(m_mutex);
        m_Channels[fd]->SetEvents(events);
        std::string ev;
        if(events&EPOLLET) ev+=" EPOLLET ";
        if(events&EPOLLIN) ev+=" EPOLLIN ";
        if(events&EPOLLOUT) ev+=" EPOLLOUT ";
        LOG_ROOT_DEBUG<<"fd: "<<fd<<" 已修改 Events: "<<ev;
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
                if(event&EPOLLIN) LOG_ROOT_DEBUG<<"fd: "<<fd<<" 读事件就绪";
                if(event&EPOLLOUT) LOG_ROOT_DEBUG<<"fd: "<<fd<<" 写事件就绪";
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
