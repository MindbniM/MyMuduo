#pragma once
#include<functional>
#include<sys/epoll.h>
#include"Socket.hpp"
#include"../buff/buffer.hpp"
#include"../fiber/Fiber.hpp"
namespace MindbniM
{
    class Channel
    {
    public:
        using ptr=std::shared_ptr<Channel>;
        using func_t=std::function<void()>;

        Channel(int fd,bool ET=false,int events=0):m_fd(fd),m_events(events)
        {
            if(ET) m_events|=EPOLLET;
        }
        void SetEventRead(){ m_events |=EPOLLIN;}
        void SetEventWrite(){ m_events |=EPOLLOUT;}
        void SetReadCall(func_t cb){_ReadCall=std::move(cb);}
        void SetWriteCall(func_t cb){_WriteCall=std::move(cb);}


        int Fd()const {return m_fd.Fd();}
        int Events()const {return m_events;}
        bool isRead()const {return m_events&EPOLLIN;}
        bool isWrite()const {return m_events&EPOLLOUT;}
    public:
        func_t  _ReadCall;
        func_t _WriteCall;

        Buffer _Out;
        Buffer _In;
    private:
        Socket m_fd;
        int m_events;

    };
}
