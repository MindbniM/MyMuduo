#pragma once
#include"InetAddr.hpp"
#include<netinet/tcp.h>
#include<unistd.h>
#include"../base/nocopyable.hpp"
namespace MindbniM
{
    class Socket : public noncopyable
    {
    public:
        explicit Socket(int fd):m_fd(fd){}
        ~Socket();
        int Fd() const {return m_fd;}
        int BindAddr(const InetAddr& addr);
        void Listen();
        int Accept(InetAddr& peer);

        //设置地址复用
        void SetReuseAddr(bool on);
        //禁用Nagle算法
        void SetTcpNoDelay(bool on);
        //设置活跃连接
        void SetKeepAlive(bool on);
    private:
        int m_fd;
    };
    Socket::~Socket()
    {
        close(m_fd);
    }
    int Socket::BindAddr(const InetAddr& addr)
    {
        int n=bind(m_fd,(struct sockaddr*)addr.GetSockAddr(),sizeof(struct sockaddr_in));
        if(n!=0)
        {
            LOG_ROOT_FATAL<<"bind error fd:"<<m_fd;
        }
        return n;
    }
    void Socket::Listen()
    {
        listen(m_fd,1024);
    }
    int  Socket::Accept(InetAddr& peer)
    {
        sockaddr_in addr;
        socklen_t len=sizeof(addr);
        memset(&addr,0,sizeof(addr));
        int fd=accept4(m_fd,(struct sockaddr*)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
        if(fd<0)
        {
            LOG_ROOT_WARNING<<"Accept error";
        }
        else
        {
            peer.SetSockAddr(addr);
            LOG_ROOT_INFO<<"new connect fd:"<<fd;
        }
        return fd;
    }
    void Socket::SetReuseAddr(bool on)
    {
         int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }
    void Socket::SetTcpNoDelay(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    }
    void Socket::SetKeepAlive(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }
}
