#pragma once
#include"Channel.hpp"
namespace MindbniM
{
    class TcpConnect : public Channel
    {
    public:
        using ptr=std::shared_ptr<TcpConnect>;
        TcpConnect(int fd,const InetAddr& addr,EventLoop* root,int BuffSize=1024):Channel(root,std::make_shared<TcpSocket>(fd,addr),true,EPOLLIN|EPOLLET),_Out(BuffSize),_In(BuffSize)
        {
            m_fd->SetNoBlock();
        }
        TcpConnect(uint16_t port,EventLoop* root):Channel(root,std::make_shared<TcpSocket>(port),true,EPOLLIN|EPOLLET),_Out(0),_In(0)
        {
            m_fd->SetNoBlock();
            std::static_pointer_cast<TcpSocket>(m_fd)->BindAddr();
            std::static_pointer_cast<TcpSocket>(m_fd)->Listen();
        }
        TcpConnect(const std::string& ip,uint16_t port,EventLoop* root,int BuffSize=1024):Channel(root,std::make_shared<TcpSocket>(InetAddr(ip,port)),true,EPOLLIN|EPOLLET),_Out(BuffSize),_In(BuffSize)
        {
            m_fd->SetNoBlock();
        }
        void Send(const std::string str);
        int SendTO(int& err);
        int Accept(InetAddr& peer,int& err);
        int Recv();
        //读取全部数据+清空缓冲区
        std::string ReAlltoBody(){return _In.Retrieve_AllToStr();}
        //不清空缓冲区读取全部数据
        std::string AlltoBody(){return {_In.Peek(),_In.Read_ableBytes()};}
        //表示已经读取len个字符, 移动输出缓冲区
        void ReadLen(size_t len){return _In.Retrieve(len);}
        size_t InBuffSize(){return _In.Read_ableBytes();}
        size_t OutBuffSize(){return _Out.Read_ableBytes();}
        
    public:
    private:
        Buffer _Out;
        Buffer _In;
    };
    int TcpConnect::SendTO(int& err)
    {
        int n=0;
        while(1)
        {
            n=_Out.WriteFd(m_fd->Fd(),&err);          
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
        _Out.Append(str);
    }
    int TcpConnect::Accept(InetAddr& peer,int& err)
    {
        return std::static_pointer_cast<TcpSocket>(m_fd)->Accept(peer,err);
    }
    int TcpConnect::Recv()
    {
        int err=0;
        int n=0;
        while(1)
        {
            n=_In.ReadFd(m_fd->Fd(),&err);
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
