#pragma once
#include<string>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
namespace MindbniM
{
    class InetAddr
    {
    public:
        explicit InetAddr(uint16_t port=0);
        explicit InetAddr(const struct sockaddr_in& addr):m_addr(addr){}
        InetAddr(const std::string& ip,uint16_t port);
    public:
        std::string Ip();
        std::string Addr();
        uint16_t  Port();
        const struct sockaddr_in* GetSockAddr();
    private:
        struct sockaddr_in m_addr;
    };
    InetAddr::InetAddr(uint16_t port)
    {
        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_family=AF_INET;
        m_addr.sin_port=htons(port);
        m_addr.sin_addr.s_addr=INADDR_ANY;
    }
    InetAddr::InetAddr(const std::string& ip,uint16_t port)
    {
        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_family=AF_INET;
        m_addr.sin_port=htons(port);
        m_addr.sin_addr.s_addr=inet_addr(ip.c_str());
    }
    std::string InetAddr::Ip()
    {
        return inet_ntoa(m_addr.sin_addr);
    }
    std::string InetAddr::Addr()
    {
        return Ip()+" : "+std::to_string(Port());
    }
    uint16_t  InetAddr::Port()
    {
        return ntohs(m_addr.sin_port);
    }
    const struct sockaddr_in* InetAddr::GetSockAddr()
    {
        return &m_addr;
    }
}
