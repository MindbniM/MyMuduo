#pragma once
#include"InetAddr.hpp"
#include<netinet/tcp.h>
#include"../buff/buffer.hpp"
#include<unistd.h>
#include<fcntl.h>
#include"../base/nocopyable.hpp"
namespace MindbniM
{
    class Socket : public noncopyable
    {
    public:
        using ptr=std::shared_ptr<Socket>;
        explicit Socket(int fd=-1):m_fd(fd){}
        virtual ~Socket();
        void SetFd(int fd){m_fd=fd;}
        int Fd() const {return m_fd;}
        void SetNoBlock();
    protected:
        int m_fd=-1;
    };
    class TcpSocket :public Socket
    {
    public:
        TcpSocket(uint16_t port);
        TcpSocket(int fd,const InetAddr& addr):Socket(fd),m_addr(addr)
        {}

        int BindAddr();
        void Listen();
        int Accept(InetAddr& peer,int& err);
        int Send(const std::string& str,int&err);
        int Recv(std::vector<char>& out,int& err);

        //设置地址复用
        void SetReuseAddr(bool on);
        //禁用Nagle算法
        void SetTcpNoDelay(bool on);
        //设置活跃连接
        void SetKeepAlive(bool on);
    private:
        InetAddr m_addr;
    };
    int TcpSocket::Send(const std::string& str,int&err)
    {
        int n=send(m_fd,str.c_str(),str.size(),MSG_NOSIGNAL);
        err=errno;
        return n;
    }
    int TcpSocket::Recv(std::vector<char>& out,int& err)
    {
        char buff[1024]={0};
        int n=recv(m_fd,buff,sizeof(buff)-1,0);
        std::vector<char> v(buff,buff+n);
        out.swap(v);
        err=errno;
        return n;
    }
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
    void Socket::SetNoBlock()
    {
        int flags=0;
        flags=fcntl(m_fd,F_GETFL,0);
        if(flags<0) return;
        flags|=O_NONBLOCK;
        fcntl(m_fd,F_SETFL,flags);
    }
}
