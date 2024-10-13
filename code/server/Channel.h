#pragma once
#include<functional>
#include<sys/epoll.h>
namespace MindbniM
{
    class EventLoop;
    class Channel
    {
    public:
        using func_t=std::function<void()>;
        Channel(EventLoop* root,int fd,int events):m_root(root),m_fd(fd),m_events(EPOLLET|events){}
        void SetEventRead(){ m_events |=EPOLLIN;}
        void SetEventWrite(){ m_events |=EPOLLOUT;}
        void SetReadCall(func_t cb){m_ReadCall=std::move(cb);}
        void SetWriteCall(func_t cb){m_WriteCall=std::move(cb);}

        void HandleEvent();
        
        int Fd()const {return m_fd;}
        int Events()const {return m_events;}
        bool isRead()const {return m_events&EPOLLIN;}
        bool isWrite()const {return m_events&EPOLLOUT;}
    private:
        EventLoop* m_root;
        int m_fd;
        int m_events;

        func_t m_ReadCall;
        func_t m_WriteCall;
    };
}
