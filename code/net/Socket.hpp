#pragma once
#include"InetAddr.hpp"
#include<netinet/tcp.h>
#include<unistd.h>
#include<fcntl.h>
#include"../base/nocopyable.hpp"
namespace MindbniM
{
    class Socket : public noncopyable
    {
    public:
        explicit Socket(int fd=-1):m_fd(fd){}
        virtual ~Socket();
        void SetFd(int fd){m_fd=fd;}
        int Fd() const {return m_fd;}
        void SetNoBlock(bool on);
    protected:
        int m_fd=-1;
    };
    class TcpSocket
    {
    public:
        TcpSocket(uint16_t port);
        TcpSocket(int fd,const InetAddr& addr):m_fd(fd),m_addr(addr)
        {}

        int BindAddr();
        void Listen();
        int Accept(InetAddr& peer,int& err);
        int Send(const std::string& str);

        //设置地址复用
        void SetReuseAddr(bool on);
        //禁用Nagle算法
        void SetTcpNoDelay(bool on);
        //设置活跃连接
        void SetKeepAlive(bool on);
        //设置非阻塞
        int Fd() const {return m_fd;}
        void SetNoBlock(bool on);
    private:
        int m_fd=-1;
        InetAddr m_addr;
    };
    Socket::~Socket()
    {
        if(m_fd>0)
        close(m_fd);
    }
    TcpSocket::TcpSocket(uint16_t port):m_addr(port)
    {
        m_fd=socket(AF_INET,SOCK_STREAM,0);
        SetReuseAddr(true);
        if(m_fd<0)
        {
            LOG_ROOT_FATAL<<"TcpSocket create error";
            exit(-1);
        }
        LOG_ROOT_INFO<<"TcpSocket create success fd:"<<m_fd;
    }
    int TcpSocket::BindAddr()
    {
        int n=bind(m_fd,(struct sockaddr*)m_addr.GetSockAddr(),sizeof(struct sockaddr_in));
        if(n!=0)
        {
            LOG_ROOT_FATAL<<"bind error fd:"<<m_fd;
        }
        LOG_ROOT_INFO<<"bind create success port:"<<m_addr.Port();
        return n;
    }
    void TcpSocket::Listen()
    {
        listen(m_fd,1024);
        LOG_ROOT_INFO<<"fd:"<<m_fd<<" listen";
    }
    int  TcpSocket::Accept(InetAddr& peer,int& err)
    {
        sockaddr_in addr;
        socklen_t len=sizeof(addr);
        memset(&addr,0,sizeof(addr));
        int fd=accept(m_fd,(struct sockaddr*)&addr,&len);
        err=errno;
        if(fd<0)
        {
            //LOG_ROOT_WARNING<<"Accept error";
        }
        else
        {
            peer.SetSockAddr(addr);
            LOG_ROOT_INFO<<"new connect fd:"<<fd;
        }
        return fd;
    }
    void TcpSocket::SetReuseAddr(bool on)
    {
         int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }
    void TcpSocket::SetTcpNoDelay(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    }
    void TcpSocket::SetKeepAlive(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }
    void Socket::SetNoBlock(bool on)
    {
        int flags=0;
        flags=fcntl(m_fd,F_GETFL,0);
        if(flags<0) return;
        flags|=O_NONBLOCK;
        fcntl(m_fd,F_SETFL,flags);
    }
    void TcpSocket::SetNoBlock(bool on)
    {
        int flags=0;
        flags=fcntl(m_fd,F_GETFL,0);
        if(flags<0) return;
        flags|=O_NONBLOCK;
        fcntl(m_fd,F_SETFL,flags);
    }
}
