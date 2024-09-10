#pragma once
#include"../log/log.hpp"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
namespace MindbniM
{
    class Socket
    {
    public:
        void init(int domain,int type,int protocol)
        {
            _fd=::socket(domain,type,protocol);
            if(_fd<0)
            {
                LOG_ROOT_FATAL("socket create error");
                exit(1);
            }
            LOG_ROOT_INFO("socket create success : %d",_fd);
        }
        void bind(const std::string& ip,uint16_t prot)
        {
            struct sockaddr_in in;
            memset(&in,0,sizeof(in));
        }
    protected:
        int _fd;
    };
}