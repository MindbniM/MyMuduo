#pragma once
#include"Channel.hpp"
namespace MindbniM
{
    class TcpConnect : public Channel
    {
    public:
        TcpConnect(int fd,const InetAddr& addr,EventLoop* root,int BuffSize=1024):Channel(root,-1,true,EPOLLIN),m_sock(fd,addr),_Out(BuffSize),_In(BuffSize)
        {
            m_sock.SetNoBlock();
        }
        TcpConnect(uint16_t port,EventLoop* root):Channel(root,-1,true,EPOLLIN),m_sock(port),_Out(0),_In(0)
        {
            m_sock.BindAddr();
            m_sock.Listen();
        }
        virtual int Fd()const {return m_sock.Fd();}
        void Send(const std::string str);
        int SendTO(int& err);
        int Accept(InetAddr& peer,int& err);
        int Recv();
    public:
    private:
        TcpSocket m_sock;
        Buffer _Out;
        Buffer _In;
    };
    int TcpConnect::SendTO(int& err)
    {
        int n=0;
        while(1)
        {
            n=_In.WriteFd(m_sock.Fd(),&err);          
            if(n<0)
            {
                if((err&EAGAIN)||(err&EWOULDBLOCK)) break;
                else 
                {
                    return n;
                }
            }
            if(n==0) return n;
        }
        return n;
    }
    void TcpConnect::Send(const std::string str)
    {
        _In.Append(str);
    }
    int TcpConnect::Accept(InetAddr& peer,int& err)
    {
        return m_sock.Accept(peer,err);
    }
    int TcpConnect::Recv()
    {
        int err=0;
        int n=0;
        while(1)
        {
            n=_Out.ReadFd(m_sock.Fd(),&err);
            if(n<0)
            {
                if((err&EAGAIN)||(err&EWOULDBLOCK)) break;
                else 
                {
                    return n;
                }
            }
            if(n==0) return n;
        }
        return n;
    }
}
