#pragma once
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<memory>
#include"../buff/buffer.hpp"
#include"TcpSocket.hpp"
namespace MindbniM
{
    class Reactor;
    class Connect
    {
    public:
        using ptr=std::shared_ptr<Connect>;
        Connect(int fd,uint32_t ip,uint16_t port,Reactor* root):_fd(fd),_nip(ip),_nport(port),_root(root)
        {}
        Connect()=default;
        virtual void send()=0;
        virtual void recv()=0;
        void set_events(uint32_t events){_events=events;}
        uint32_t get_events(){return _events;}
        int port()
        {
            return ntohs(_nport);
        }
        std::string ip()
        {
            struct in_addr in;
            in.s_addr=_nip;
            return std::string(inet_ntoa(in));
        }
        int fd(){return _fd;}
        void set_root(Reactor* root)
        {
            _root=root;
        }
    protected:
        int _fd;
        uint32_t _nip;
        uint16_t _nport; 
        uint32_t _events;
        Reactor* _root=nullptr;
    };
    class Conn : public Connect
    {
    public:
        Conn(int fd,uint32_t ip,uint16_t port,Reactor* root):Connect(fd,ip,port,root)
        {}
        void send()override;
        void recv()override;
    private:
        Buffer _out;
        Buffer _in;
    };
    class Listen : public Connect
    {
    public:
        Listen(uint16_t port)
        {
            m_sock.init(port);
            _fd=m_sock.fd();
            _nport=htons(port);
        }
        void send()override;
        void recv()override;
    private:
        Connect::ptr accept();
        Tcp_Server_Socket m_sock;        
    };
    Connect::ptr Listen::accept()
    {
        struct sockaddr_in in;
        socklen_t len=sizeof(in);
        int fd=::accept(_fd,(struct sockaddr*)&in,&len);
        if(fd<0)
        {
            LOG_ROOT_WARNING("listen accept error");
        }
        LOG_ROOT_INFO("listen accpet success addr:%s:%d",inet_ntoa(in.sin_addr),ntohl(in.sin_port));
        return std::make_shared<Conn>(fd,in.sin_addr.s_addr,in.sin_port,_root);
    }
    void Conn::send()
    {
        std::string str(_out.begin(),_out.end());
        int n=::send(_fd,str.c_str(),str.size(),0);
    }
    void Conn::recv()
    {}
    void Listen::send()
    {
        Connect::ptr p=accept();
        //p->set_events();
        _root->AddConnect(p);
    }
    void Listen::recv()
    {
        Connect::ptr p=accept();
        //p->set_events();
        _root->AddConnect(p);
    }

}