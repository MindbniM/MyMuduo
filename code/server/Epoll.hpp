#include"Connect.hpp"
#include"../log/log.hpp"
#include<sys/epoll.h>
namespace MindbniM
{
    template<size_t N=64>
    class Epoll
    {
    public:
        void Epoll_init()
        {
            _fd=epoll_create(1);
            if(_fd<0)
            {
                LOG_ROOT_FATAL("epoll create error");
                exit(1);
            }
            LOG_ROOT_INFO("epoll create success fd:%d",_fd);
        }
        int size wait(int timeout=-1)
        {
            return ::epoll_wait(_fd,&m_event,N,timeout);
        }
        void push(Connect::ptr con)
        {
            epoll_ctl(_fd,)
        }
        epoll_event& operator[](int size)
        {
            return m_event[size];
        }
    private:
        int m_fd;
        struct epoll_event m_event[N];
    };
}