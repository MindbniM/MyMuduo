#pragma once
#include<cstdlib>
#include<memory>
#include<string>
#include<functional>
#include<thread>
#include<ucontext.h>
#include<atomic>
#include<utility>
#include<type_traits>
#include"../log/log.hpp"
namespace MindbniM
{
    //全局变量, 协程id
    std::atomic<uint64_t> s_fiber_id(0);
    //全局变量, 协程数量
    std::atomic<uint64_t> s_fiber_num(0);
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        using func_t=std::function<void()>;
        using ptr=std::shared_ptr<Fiber>;
        enum class state
        {
          INIT,   //初始化
          HOLD,   //挂起
          EXEC,   //运行
          TERM,   //结束
          READY,  //就绪
        };
    public:
        Fiber(func_t cb,bool isrunInSchedule=true,uint32_t stack_size=128*1024);
        template<class Fn,class... Args>
        Fiber(Fn&& func, Args&&... args);
        ~Fiber();

        //重置协程函数,重置状态INIT/TERM,复用栈空间
        void reset(func_t cb);
        //切换到当前协程运行
        void swapIn();
        //切换到后台运行
        void swapOut();
        //调用协程
        void call();
        //获取协程状态
        state getState(){ return m_s;}
    public:
        //设置一个协程为当前协程
        static void SetThis(Fiber*f);
        //获取当前执行协程的智能指针
        static Fiber::ptr Get_this();
        //当前运行协程切换到后台, 并设置为Ready状态
        static void YeildtoReady();
        //当前运行协程切换到后台, 并设置为Hold状态
        static void YeildtoHold();
        //获取总协程数
        static uint64_t GetFiberNum();
        static void MainFunc();
    private:
        //用于创建主协程
        Fiber();
    private:
        uint64_t m_id=0;            //协程id
        uint32_t m_stack_size=0;    //协程栈空间大小
        ucontext_t m_ctx;           //协程上下文
        void* m_stack=nullptr;      //协程使用的栈
        state m_s=state::INIT;      //协程状态
        func_t m_cb;                //协程具体回调
        bool m_runInSchedule;       //是否参与调度器调度
    };
    //线程当前执行的协程
    thread_local Fiber* t_fiber=nullptr;
    //线程的主协程
    thread_local Fiber::ptr t_thread_fiber=nullptr;



    //统一的方法来处理协程栈空间的分配
    class MallocStackAllocator 
    {
    public:
        static void* Alloc(size_t size) {
            return malloc(size);
        }
        static void Dealloc(void* vp, size_t size) {
            return free(vp);
        }
    };
    using StackAllocator=MallocStackAllocator;

    Fiber::Fiber(func_t cb,bool isrunInSchedule,uint32_t stack_size):m_id(s_fiber_id++),m_stack_size(stack_size),m_cb(cb),m_runInSchedule(isrunInSchedule)
    {
        ++s_fiber_num;
        m_stack=StackAllocator::Alloc(m_stack_size);
        getcontext(&m_ctx);
        m_ctx.uc_link=nullptr;
        m_ctx.uc_stack.ss_sp=m_stack;
        m_ctx.uc_stack.ss_size=m_stack_size;
        makecontext(&m_ctx,&Fiber::MainFunc,0);
    }
    template<class Fn,class... Args>
    Fiber::Fiber(Fn&& func, Args&&... args):m_id(s_fiber_id++),m_stack_size(1024*1024),
      m_cb([=]{auto f=std::bind(func,args...);f(); }),m_runInSchedule(true)               
    {
        ++s_fiber_num;
        m_stack=StackAllocator::Alloc(m_stack_size);
        getcontext(&m_ctx);
        m_ctx.uc_link=nullptr;
        m_ctx.uc_stack.ss_sp=m_stack;
        m_ctx.uc_stack.ss_size=m_stack_size;
        makecontext(&m_ctx,&Fiber::MainFunc,0);
    }
    Fiber::Fiber()
    {
        SetThis(this);
        getcontext(&m_ctx);
        m_s=state::EXEC;
        m_id=s_fiber_id;
        ++s_fiber_num;
        ++s_fiber_id;
    }
    Fiber::~Fiber()
    {
        --s_fiber_num;
        if(m_stack!=nullptr)
        {
            StackAllocator::Dealloc(m_stack,m_stack_size);
        }
        else
        {
            Fiber* p=t_fiber;
            if(p==this)
            {
                SetThis(nullptr);
            }
        }
    }
    void Fiber::SetThis(Fiber*f)
    {
        t_fiber=f;
    }
    Fiber::ptr Fiber::Get_this()
    {
        if(t_fiber!=nullptr)
        {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        t_thread_fiber=main_fiber;
        return main_fiber->shared_from_this();
    }
    void Fiber::reset(func_t cb)
    {
        m_cb=cb;
        getcontext(&m_ctx);
        m_ctx.uc_link=nullptr;
        m_ctx.uc_stack.ss_sp=m_stack;
        m_ctx.uc_stack.ss_size=m_stack_size;
        makecontext(&m_ctx,&Fiber::MainFunc,0);
        m_s=state::INIT;
    }
    void Fiber::call()
    {
      m_s=state::EXEC;
      swapcontext(&t_thread_fiber->m_ctx,&m_ctx);
    }
    void Fiber::YeildtoReady()
    {
        Fiber::ptr p=Get_this();
        p->m_s=state::READY;
        p->swapOut();
    }
    void Fiber::YeildtoHold()
    {
        Fiber::ptr p=Get_this();
        p->m_s=state::HOLD;
        p->swapOut();
    }

    uint64_t GetFiberNum()
    {
        return s_fiber_num;
    }
    void Fiber::swapIn()
    {
        SetThis(this);
        swapcontext(&t_thread_fiber->m_ctx,&m_ctx);
    }
    void Fiber::swapOut()
    {
        SetThis(t_thread_fiber.get());
        swapcontext(&m_ctx,&t_thread_fiber->m_ctx);
    }
    void Fiber::MainFunc()
    {
        Fiber::ptr p=Get_this();
        p->m_cb();
        p->m_cb=nullptr;
        p->m_s=state::TERM;
        auto cur=p.get();
        p.reset();
        cur->swapOut();
    }

}
