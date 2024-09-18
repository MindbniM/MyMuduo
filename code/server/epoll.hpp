#pragma once
#include <vector>
#include<cstring>
#include <sys/epoll.h>
#include "../log/log.hpp"
namespace MindbniM
{
    class Epoll
    {
    public:
        explicit Epoll(int maxEvent = 1024):_fd(epoll_create(128)),_v(maxEvent)
        {}

        ~Epoll()
        {
            ::close(_fd);
        }

        bool AddFd(int fd, uint32_t events);

        bool ModFd(int fd, uint32_t events);

        bool DelFd(int fd);

        int Wait(int timeoutMs = -1);

        int GetEventFd(size_t i) const;

        uint32_t GetEvents(size_t i) const;

    private:
        int _fd;
        std::vector<struct epoll_event> _v;
    };

    bool Epoll::AddFd(int fd, uint32_t events)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=events;
        event.data.fd=fd;
        epoll_ctl(_fd,EPOLL_CTL_ADD,fd,&event);
        return true;
    }

    bool Epoll::ModFd(int fd, uint32_t events)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=events;
        event.data.fd=fd;
        epoll_ctl(_fd,EPOLL_CTL_MOD,fd,&event);
        return true;
    }

    bool Epoll::DelFd(int fd)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        epoll_ctl(_fd,EPOLL_CTL_DEL,fd,&event);
    }

    int Epoll::Wait(int timeoutMs = -1)
    {
        return epoll_wait(_fd,&_v[0],static_cast<int>(_v.size()),timeoutMs);
    }

    int Epoll::GetEventFd(size_t i) const
    {
        if(i<0||i>=_v.size()) return -1;
        return _v[i].data.fd;
    }

    uint32_t Epoll::GetEvents(size_t i) const
    {
        if(i<0||i>=_v.size()) return -1;
        return _v[i].events;
    }

}