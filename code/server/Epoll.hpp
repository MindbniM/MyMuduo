#pragma once
#include<sys/epoll.h>
#include"../log/log.hpp"
#include"Connect.hpp"
namespace MindbniM
{
    template<size_t N=64>
    class Epoll
    {
    public:
        void Epoll_init()
        {
            m_fd=epoll_create(1);
            if(m_fd<0)
            {
                LOG_ROOT_FATAL("epoll create error");
                exit(1);
            }
            LOG_ROOT_INFO("epoll create success fd:%d",m_fd);
        }
        int wait(int timeout=-1)
        {
            return ::epoll_wait(m_fd,m_event,N,timeout);
        }
        epoll_event& operator[](int size)
        {
            return m_event[size];
        }
        void add(Connect::ptr con)
        {
            ctl(con,EPOLL_CTL_ADD);
        }
        void del(Connect::ptr con)
        {
            ctl(con,EPOLL_CTL_DEL);
        }
        void mod(Connect::ptr con)
        {
            ctl(con,EPOLL_CTL_MOD);
        }
    private:
        void ctl(Connect::ptr con,int op)
        {
            struct epoll_event event;
            event.events=con->get_events();
            event.data.fd=con->fd();
            int n=epoll_ctl(m_fd,op,con->fd(),&event);
            if(n<0)
            {
                LOG_ROOT_ERROR("epoll ctl error");
            }
        }
        int m_fd;
        struct epoll_event m_event[N];
    };
}