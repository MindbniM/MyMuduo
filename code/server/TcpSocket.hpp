#pragma once
#include"../log/log.hpp"
#include"Epoll.hpp"
#include"Connect.hpp"
#include<string.h>
namespace MindbniM
{
    class Socket
    {
    public:
        void init(int domain,int type,int protocol);
    protected:
        int _fd;
    };
    class Tcp_Server_Socket : public Socket
    {
    public:
        void init(uint16_t port);
        void listen(int size=8);
    };
    class Listen_Socket : public Tcp_Server_Socket
    {
    public:
        Connect::ptr accept();
    };
    class Tcp_Client_Socket : public Socket
    {

    };
    void Socket::init(int domain,int type,int protocol) 
    {
        _fd=::socket(domain,type,protocol);
        if(_fd<0)
        {
            LOG_ROOT_FATAL("socket create error");
            exit(1);
        }
        LOG_ROOT_INFO("socket create success fd: %d",_fd);
    }
    void Tcp_Server_Socket::init(uint16_t port)
    {
        Socket::init(AF_INET,SOCK_STREAM,0);
        struct sockaddr_in in;
        in.sin_family=AF_INET;
        in.sin_addr.s_addr=INADDR_ANY;
        in.sin_port=htons(port);
        int n=::bind(_fd,(struct sockaddr*)&in,sizeof(in));
        if(n<0)
        {
            LOG_ROOT_FATAL("bind error errno:%d",errno);
            exit(1);
        }
        LOG_ROOT_INFO("bind success prot:%d",in.sin_port);
    }
    void Tcp_Server_Socket::listen(int size)
    {
        int n=::listen(_fd,size);
        if(n<0)
        {
            LOG_ROOT_FATAL("listen error errno:%d",errno);
            exit(1);
        }
        LOG_ROOT_INFO("listen success");
    }
    Connect::ptr Listen_Socket::accept()
    {
        struct sockaddr_in in;
        socklen_t len=sizeof(in);
        int fd=::accept(_fd,(struct sockaddr*)&in,&len);
        if(fd<0)
        {
            LOG_ROOT_WARNING("accept error");
        }
        LOG_ROOT_INFO("accpet success addr:%s:%d",inet_ntoa(in.sin_addr),ntohl(in.sin_port));
        return std::make_shared<Connect>(_fd,in.sin_addr.s_addr,in.sin_port);
    }
}