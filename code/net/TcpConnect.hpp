#pragma once
#include"Channel.hpp"
namespace MindbniM
{
    class TcpConnect : public Channel
    {
    public:
        TcpConnect(int fd,const InetAddr& addr,EventLoop* root):Channel(root,-1,true,EPOLLIN),m_sock(fd,addr)
        {}
        Buffer& OutBuffer(){return _Out;};
        Buffer& InBuffer(){return _In;};
        int Fd()const {return m_sock.Fd();}
    public:
    private:
        TcpSocket m_sock;
        Buffer _Out;
        Buffer _In;
    };
}
