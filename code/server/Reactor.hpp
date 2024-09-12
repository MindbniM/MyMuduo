#pragma once
#include"Epoll.hpp"
#include<unordered_map>
namespace MindbniM
{
    class Reactor
    {
    public:
        void AddConnect(Connect::ptr con)
        {
            if(!IsExits(con->fd()))
            {
                _Connects[con->fd()]=con;
                _epoll.add(con);
            }
        }
        void DelConnect(Connect::ptr con)
        {
            auto it=_Connects.find(con->fd());
            if(it!=_Connects.end())
            {
                _Connects.erase(it);
            }
        }
        void start(int timeout=-1)
        {
            while(1)
            {
                int n=_epoll.wait(timeout);
                for(int i=0;i<n;i++)
                {
                    uint32_t events=_epoll[i].events;
                    int fd=_epoll[i].data.fd;
                    if(events&EPOLLHUP) events|=(EPOLLIN|EPOLLOUT);
                    if(events&EPOLLERR) events|=(EPOLLIN|EPOLLOUT);
                    if(events&EPOLLIN)
                    {
                        if(IsExits(fd))
                        {
                            _Connects[fd]->recv();
                        }
                    }
                    else if(events&EPOLLOUT)
                    {
                        if(IsExits(fd))
                        {
                            _Connects[fd]->send();
                        }
                    }
                }

            }
        }
    private:
        bool IsExits(int fd)
        {
            return _Connects.count(fd);
        }
        std::unordered_map<int,Connect::ptr> _Connects;
        Epoll<> _epoll;
    };
}