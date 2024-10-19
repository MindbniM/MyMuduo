#pragma once
#include<functional>
#include<sys/epoll.h>
#include"Socket.hpp"
#include"../buff/buffer.hpp"
#include"../fiber/Fiber.hpp"
namespace MindbniM
{
    class EventLoop;
    class Channel
    {
    public:
        using ptr=std::shared_ptr<Channel>;
        using func_t=std::function<void()>;

        Channel(EventLoop* root,int fd,bool ET=false,int events=0):m_fd(new Socket(fd)),m_events(events),m_root(root)
        {
            if(ET) m_events|=EPOLLET;
        }
        Channel(EventLoop* root,Socket::ptr fd,bool ET=false,int events=0):m_fd(fd),m_events(events),m_root(root)
        {
            if(ET) m_events|=EPOLLET;
        }
        void SetEventRead(){ m_events |=EPOLLIN;}
        void SetEventWrite(){ m_events |=EPOLLOUT;}
        void SetReadCall(func_t cb){_ReadCall=std::move(cb);}
        void SetWriteCall(func_t cb){_WriteCall=std::move(cb);}
        void SetEvents(int events){m_events=events;}
        void ReEvents(){m_events=0;}


        int Fd()const {return m_fd->Fd();}
        EventLoop* Root(){return m_root;}
        int Events()const {return m_events;}
        bool isRead()const {return m_events&EPOLLIN;}
        bool isWrite()const {return m_events&EPOLLOUT;}
    public:
        func_t  _ReadCall;
        func_t _WriteCall;

    protected:
        EventLoop* m_root;
        int m_events;
        Socket::ptr  m_fd;

    };
}
