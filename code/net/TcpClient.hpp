#pragma once
#include"Socket.hpp"
#include"TcpConnect.hpp"
#include"EventLoop.hpp"
namespace MindbniM
{
    class TcpClient
    {
    public:
        using CallBack=std::function<void(TcpConnect::ptr)>;
        TcpClient(const std::string& ip,uint16_t port,int WriteCall=2);
        void SetMessageCallBack(CallBack cb){_MessageCallBack=std::move(cb);}
        void Send(const std::string& str);
        ~TcpClient();

    private:
        void WriteCall(Channel::ptr con);
        void ReadCall(Channel::ptr con);
        void AddWork(Channel::func_t);
    private:
        EventLoop::ptr m_loop;
        std::thread m_loopt;
        TcpConnect::ptr m_sock;
        CallBack _MessageCallBack;
        Schedule::ptr m_sche;
    };
    TcpClient::TcpClient(const std::string& ip,uint16_t port,int WorkNum):m_loop(std::make_shared<EventLoop>()),m_sock(std::make_shared<TcpConnect>(ip,port,m_loop.get()))
    {
        m_sche=std::make_shared<Schedule>(WorkNum,false);
        Channel::func_t cb=std::bind(&TcpClient::ReadCall,this,m_sock);
        m_sock->SetReadCall(std::bind(&TcpClient::AddWork,this,cb));
        cb=std::bind(&TcpClient::WriteCall,this,m_sock);
        m_sock->SetWriteCall(std::bind(&TcpClient::AddWork,this,cb));
        m_loop->insert(m_sock);
        m_loopt=std::thread(&EventLoop::Loop,m_loop.get(),-1);
        m_sche->start();
    }
    void TcpClient::Send(const std::string& str)
    {
        m_sock->Send(str);
        int err;
        int n=m_sock->SendTO(err);
        if(n==0)
        {
            LOG_ROOT_DEBUG<<"服务端可能已关闭";
            return ;
        }
        else if(n<0&&((err&EAGAIN)||(err&EWOULDBLOCK)))
        {
            m_sock->ReEvents();
            m_sock->SetEventRead();
            m_sock->SetEventWrite();
            m_loop->mod(m_sock->Fd(),m_sock->Events());
        }
        
    }
    void TcpClient::AddWork(Channel::func_t cb)
    {
        Fiber::ptr f=std::make_shared<Fiber>(cb);
        m_sche->schedule(f);
    }
    void TcpClient::ReadCall(Channel::ptr con)
    {
        int n=std::static_pointer_cast<TcpConnect>(con)->Recv();
        if(n==0)
        {
            LOG_ROOT_DEBUG<<"连接 fd:"<<con->Fd()<<" 可能已关闭";
            con->Root()->erase(con->Fd());
            return ;
        }
        if(_MessageCallBack) _MessageCallBack(std::static_pointer_cast<TcpConnect>(con));
        int err;
        n=std::static_pointer_cast<TcpConnect>(con)->SendTO(err);
        if(n<0&&((err&EAGAIN)||(err&EWOULDBLOCK)))
        {
            con->SetEventWrite();
            con->Root()->mod(con->Fd(),con->Events());
            LOG_ROOT_DEBUG<<"fd:"<<con->Fd()<<" add EPOLLOUT";
        }
    }
    void TcpClient::WriteCall(Channel::ptr con)
    {
        int err;
        int n=std::static_pointer_cast<TcpConnect>(con)->SendTO(err);
        if(n==0)
        {
            LOG_ROOT_DEBUG<<"连接 fd:"<<con->Fd()<<" 可能已关闭";
            con->Root()->erase(con->Fd());
            return ;
        }
        else if(n<0&&(err&EAGAIN)||(err&EWOULDBLOCK))
        {
            con->ReEvents();
            con->SetEventRead();
            con->SetEventWrite();
            con->Root()->mod(con->Fd(),con->Events());
        }
        
    }
    TcpClient::~TcpClient()
    {
        m_sche->stop();
        m_loopt.join();
    }
}
