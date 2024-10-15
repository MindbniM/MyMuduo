#pragma once
#include <vector>
#include<cstring>
#include <sys/epoll.h>
#include<unistd.h>
#include "../log/log.hpp"
namespace MindbniM
{
    class Epoll
    {
    public:
        explicit Epoll(int maxEvent = 1024):m_epoll_fd(epoll_create(128)),_v(maxEvent)
        {}

        ~Epoll()
        {
            ::close(m_epoll_fd);
        }

        bool AddFd(int fd, uint32_t events);

        bool ModFd(int fd, uint32_t events);

        bool DelFd(int fd);

        int Wait(int timeoutMs = -1);

        int GetEventFd(size_t i) const;

        uint32_t GetEvents(size_t i) const;

    private:
        int m_epoll_fd;
        std::vector<struct epoll_event> _v;
    };

    bool Epoll::AddFd(int fd, uint32_t events)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=events;
        event.data.fd=fd;
        epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,fd,&event);
        return true;
    }

    bool Epoll::ModFd(int fd, uint32_t events)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=events;
        event.data.fd=fd;
        epoll_ctl(m_epoll_fd,EPOLL_CTL_MOD,fd,&event);
        return true;
    }

    bool Epoll::DelFd(int fd)
    {
        if(fd<0) return false;
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,fd,&event);
        return true;
    }

    int Epoll::Wait(int timeoutMs)
    {
        return epoll_wait(m_epoll_fd,&_v[0],static_cast<int>(_v.size()),timeoutMs);
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
