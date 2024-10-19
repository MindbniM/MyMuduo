#ifndef __SCHEDULE__
#define __SCHEDULE__
#include<condition_variable>
#include"Fiber.hpp"
namespace MindbniM
{
    
    struct ScheduleTask
    {
        Fiber::ptr fiber;         //可使用协程调度
        std::function<void()> cb; //可使用函数调用
        ScheduleTask()=default;
        ScheduleTask(Fiber::ptr f)
        {
            fiber=f;
        }
        ScheduleTask(std::function<void()> func)
        {
            cb=func;
        }
        void reset()
        {
            fiber=nullptr;
            cb=nullptr;
        }
    };
    using TaskQueue=std::queue<ScheduleTask>;
    class Schedule
    {
    public:      
        using ptr=std::shared_ptr<Schedule>;
        Schedule(size_t thread_num=1,bool userCall=true,const std::string& name="Schedule");
        virtual void run();
        virtual ~Schedule();
        const std::string& GetName()const{return m_name;}
        //获得当前调度器指针
        static Schedule* Getthis();
        //获取当前线程的主协程
        static Fiber* GetMainFiber();
        //添加任务
        template<class FiberOrCb>
        void schedule(FiberOrCb cb);
        //唤醒线程
        virtual void tickle();
        //启动调度器
        void start();
        //停止调度器
        void stop();
        //设置当前调度器
        void SetThis();
        //wait
        int idleNum()const {return m_idleThreadCount;}
    private:
        template<class FiberOrCb>
        bool _schedule(FiberOrCb cb);
    private:
        std::string m_name;                           //调度器id
        std::mutex m_mutex;                           //互斥锁
        std::condition_variable  m_cond;              //条件变量
        TaskQueue m_task;                             //任务队列
        std::vector<std::thread> m_threads;           //线程池
        int m_threadCount=0;                          //工作线程数
        std::atomic<size_t> m_activeThreadCount={0};  //活跃线程数
        std::atomic<size_t> m_idleThreadCount={0};    //idle线程数
        bool m_userCall;                              //是否将当前线程作为调度线程
        Fiber::ptr m_rootFiber;                       //如果启用userCall, 当前线程的调度协程
        uint64_t m_rootId;                            //启用userCall, 当前现场的id;
        bool m_stop=false;                            //停止调度器
    };
    //当前线程的调度器
    thread_local Schedule* t_schedule=nullptr;
    //当前线程的调度协程
    thread_local Fiber* t_schedule_fiber=nullptr;


    Schedule::Schedule(size_t thread_num,bool userCall,const std::string& name):m_name(name),m_userCall(userCall)
    {
        LOG_ROOT_DEBUG<<"Schedule create";
        if(userCall)
        {
            //给当前线程创建主协程
            Fiber::Get_this();
            --thread_num;
            t_schedule=this;
            //给当前线程创建调度协程
            m_rootFiber.reset(new Fiber(std::bind(&Schedule::run,this)));
            t_schedule_fiber=m_rootFiber.get();
            m_rootId=pthread_self();
        }
        m_threadCount=thread_num;
    }
    Schedule* Schedule::Getthis()
    {
        return t_schedule;
    }
    Fiber* Schedule::GetMainFiber()
    {
        return t_schedule_fiber;
    }
    template<class FiberOrCb>
    bool Schedule::_schedule(FiberOrCb cb)
    {
        bool need_tickle=m_task.empty();
        ScheduleTask t(cb);
        if(t.cb||t.fiber)
        {
            m_task.push(t);
        }
        return need_tickle;
    }
    template<class FiberOrCb>
    void Schedule::schedule(FiberOrCb cb)
    {
        bool need_tickle=false;
        {
            std::lock_guard<std::mutex> g(m_mutex);
            need_tickle=_schedule(cb);
        }
        if(need_tickle)
        {
            tickle();
        }
    }
    void Schedule::tickle()
    {
        m_cond.notify_all();
    }
    void Schedule::start()
    {
        LOG_ROOT_INFO<<"Schedule 启动";
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if(m_stop)
            {
                LOG_ROOT_DEBUG<<"Schedule 停止";
                return;
            }
            m_threads.resize(m_threadCount);
            for(int i=0;i<m_threadCount;i++)
            {
                m_threads[i]=std::thread(std::bind(&Schedule::run,this));
            }

        }
        if(m_userCall)
        {
            
            m_rootFiber->swapIn();
        }
    }
    void Schedule::run()
    {
        LOG_ROOT_DEBUG<<"工作线程"<<pthread_self()<<" 启动";
        SetThis();
        if(pthread_self()!=m_rootId)
        {
            t_schedule_fiber=Fiber::Get_this().get();
        }
        ScheduleTask task;
        while(1)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if(pthread_self()==m_rootId&&m_task.empty())
                {
                    lock.unlock();
                    t_thread_fiber->swapIn();
                }
                while(m_task.empty()&&!m_stop)
                {
                    ++m_idleThreadCount;
                    LOG_ROOT_DEBUG<<"线程"<<pthread_self()<<"wait";
                    m_cond.wait(lock);
                    LOG_ROOT_DEBUG<<"线程"<<pthread_self()<<"work";
                    --m_idleThreadCount;
                }
                if(m_task.empty()&&m_stop)
                {
                    break;
                }
                LOG_ROOT_DEBUG<<"线程"<<pthread_self()<<"获得任务";
                task=m_task.front();
                m_task.pop();
                ++m_activeThreadCount;
            }
            if(task.cb||task.fiber)
            {
                if(task.cb)
                {
                    task.cb();
                }
                if(task.fiber)
                {
                    LOG_ROOT_DEBUG<<"切换到任务协程";
                    task.fiber->swapIn();
                    LOG_ROOT_DEBUG<<"切换回调度协程";
                }
                task.reset();
                --m_activeThreadCount;
            }
            if(!m_task.empty()&&m_idleThreadCount>0)
            {
                tickle();
            }
        }
        
    }
    void Schedule::stop()
    {
        m_stop=true;
        for(auto& t:m_threads)
        {
            t.join();
        }
    }
    void Schedule::SetThis()
    {
        t_schedule=this;
    }
    Schedule::~Schedule()
    {
        t_schedule=nullptr;
    }
}
#endif
