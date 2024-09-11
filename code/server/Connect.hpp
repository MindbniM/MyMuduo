#pragma once
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<memory>
#include"../buff/buffer.hpp"
namespace MindbniM
{
    class Connect
    {
    public:
        using ptr=std::shared_ptr<Connect>;
        Connect(int fd,uint32_t ip,uint16_t port):m_fd(fd),m_nip(ip),m_nport(port)
        {}
        int port()
        {
            return ntohs(m_nport);
        }
        std::string ip()
        {
            struct in_addr in;
            in.s_addr=m_nip;
            return std::string(inet_ntoa(in));
        }
    private:
        int m_fd;
        uint32_t m_nip;
        uint16_t m_nport; 
        Buffer m_buff;
    };

}