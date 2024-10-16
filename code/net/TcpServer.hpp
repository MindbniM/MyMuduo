#pragma once
#include "EventLoop.hpp"
#include"../fiber/Schedule.hpp"
namespace MindbniM
{
    class TcpServer
    {
    public:
        TcpServer(uint16_t port,int PollNum=2,int WorkNum=2);
        void start(int timeout=-1);
        void stop();
        ~TcpServer();
    private:
        void AddWork(Channel::func_t);
        void Accept();
        int getnext();

    private:
        TcpSocket m_listen;
        EventLoop::ptr m_root;
        std::vector<EventLoop::ptr> m_loops;
        std::vector<std::thread> m_loopthreads;
        int m_PollNum;
        Schedule::ptr m_sche;
    };
    TcpServer::TcpServer(uint16_t port,int PollNum,int WorkNum):m_listen(port),m_PollNum(PollNum),m_loops(PollNum),m_loopthreads(PollNum)
    {
        m_listen.BindAddr();
        m_listen.Listen();
        m_root=std::make_shared<EventLoop>();
        for(int i=0;i<PollNum;i++)
        {
            m_loops[i]=std::make_shared<EventLoop>();
        }
        m_sche=std::make_shared<Schedule>(WorkNum,false);
        std::function<void()> i=std::bind(&TcpServer::Accept,this);
        m_root->insert(m_listen.Fd(),EPOLLET|EPOLLIN,std::bind(&TcpServer::AddWork,this,i));
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
        }
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
    void TcpServer::Accept()
    {
        int err;
        InetAddr peer;
        int fd;
        while(1)
        {
            fd=m_listen.Accept(peer,err);
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
            m_loops[i]->insert(fd,EPOLLIN|EPOLLET);
            
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
