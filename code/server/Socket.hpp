#pragma once
#include"InetAddr.hpp"
#include"../nocopyable/nocopyable.hpp"
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
    private:
        int m_fd;
    };
}
