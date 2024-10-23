#pragma once
#include"TcpConnect.hpp"
#include "EventLoop.hpp"
#include"../fiber/Schedule.hpp"
namespace MindbniM
{
    class TcpServer
    {
    public:
        using CallBack=std::function<void(TcpConnect::ptr)>;
        TcpServer(uint16_t port,int PollNum=2,int WorkNum=2);
        void start(int timeout=-1);
        void stop();
        void SetAcceptCallBack(CallBack cb){_AcceptCallBack=cb;}
        void SetMessageCallBack(CallBack cb){_MessageCallBack=cb;}
    private:
        void AddWork(Channel::func_t);
        void AcceptCall(Channel::ptr con);
        void ReadCall(Channel::ptr con);
        void WriteCall(Channel::ptr con);

        int getnext();
    private:
        EventLoop::ptr m_root;
        std::vector<EventLoop::ptr> m_loops;
        std::vector<std::thread> m_loopthreads;
        int m_PollNum;
        Schedule::ptr m_sche;

        CallBack _AcceptCallBack;
        CallBack _MessageCallBack;
    };
    TcpServer::TcpServer(uint16_t port,int PollNum,int WorkNum):m_PollNum(PollNum),m_loops(PollNum),m_loopthreads(PollNum)
    {
        m_root=std::make_shared<EventLoop>();
        for(int i=0;i<PollNum;i++)
        {
            m_loops[i]=std::make_shared<EventLoop>();
        }
        m_sche=std::make_shared<Schedule>(WorkNum,false);
        Channel::ptr p=std::make_shared<TcpConnect>(port,m_root.get());
        std::function<void()> cb=std::bind(&TcpServer::AcceptCall,this,p);
        p->SetReadCall(std::bind(&TcpServer::AddWork,this,cb));
        m_root->insert(p);
    }
    void TcpServer::AddWork(Channel::func_t f)
    {
        Fiber::ptr p=std::make_shared<Fiber>(f);
        m_sche->schedule(p);
    }
    void TcpServer::start(int timout)
    {
        for(int i=0;i<m_PollNum;i++)
        {
            m_loopthreads[i]=std::thread(&EventLoop::Loop,m_loops[i].get(),timout);
            LOG_ROOT_INFO<<"id: "<<i<<" 子Reactor 启动";
        }
        m_sche->start();
        LOG_ROOT_INFO<<"主Reactor 启动";
        m_root->Loop(timout);
    }
    void TcpServer::stop()
    {
        for(int i=0;i<m_PollNum;i++)
        {
            m_loops[i]->stop();
        }
        m_sche->stop();
        for(int i=0;i<m_PollNum;i++)
        {
            m_loopthreads[i].join();
        }
    }
    void TcpServer::AcceptCall(Channel::ptr p)
    {
        int err;
        InetAddr peer;
        int fd;
        while(1)
        {
            fd=std::static_pointer_cast<TcpConnect>(p)->Accept(peer,err);
            if(fd<0)
            {
                if((err&EAGAIN)||(err&EWOULDBLOCK))
                {
                }
                else 
                {
                    LOG_ROOT_WARNING<<"Accept error";
                }
                return ;
            }
            int i=getnext();
            LOG_ROOT_DEBUG<<"新连接 fd: "<<fd<<"  ip: "<<peer.Addr()<<" 分配给子Reactor id:"<<i;
            Channel::ptr newConnect=std::make_shared<TcpConnect>(fd,peer,m_loops[i].get());
            std::function<void()> cb=std::bind(&TcpServer::ReadCall,this,newConnect);
            newConnect->SetReadCall(std::bind(&TcpServer::AddWork,this,cb));
            cb=std::bind(&TcpServer::WriteCall,this,newConnect);
            newConnect->SetWriteCall(std::bind(&TcpServer::AddWork,this,cb));
            newConnect->SetEventRead();
            m_loops[i]->insert(newConnect);
            if(_AcceptCallBack) _AcceptCallBack(std::static_pointer_cast<TcpConnect>(newConnect));
        }
    }
    void TcpServer::ReadCall(Channel::ptr con)
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
    void TcpServer::WriteCall(Channel::ptr con)
    {
        int err;
        int n=std::static_pointer_cast<TcpConnect>(con)->SendTO(err);
        if(n==0)
        {
            LOG_ROOT_DEBUG<<"连接 fd:"<<con->Fd()<<" 可能已关闭";
            con->Root()->erase(con->Fd());
            return ;
        }
        else if(n<0&&((err&EAGAIN)||(err&EWOULDBLOCK)))
        {
            con->ReEvents();
            con->SetEventRead();
            con->SetEventWrite();
            con->Root()->mod(con->Fd(),con->Events());
        }
        
    }
    int TcpServer::getnext()
    {
        static std::atomic<int> i(0);
        int temp=i++;
        if(i>=m_PollNum) i=0;
        return temp;
    }
}
