#pragma once
#include<string>
#include<memory>
#include<list>
#include<iostream>
#include<fstream>
#include<sstream>
namespace MindbniM
{
    //日志事件
    class LogEvent
    {
    public:
        using ptr=std::shared_ptr<LogEvent>;
        LogEvent();
    private:
        std::string m_file;     //文件名
        int m_line;             //行号
        uint32_t m_threadId;    //线程id
        uint32_t m_fiberId;     //协程id
        uint64_t m_time;        //时间戳
        std::string m_message;  //消息
    };
    //日志等级
    class LogLevel
    {
    public:
        enum Level
        {
            DEBUG=1,
            INFO=2,
            WARNING=3,
            ERROR=4,
            FATAL=5
        };
    };
    //日志格式器
    class LogFormatter
    {
    public:
        using ptr=std::shared_ptr<LogFormatter>;
        std::string format(LogEvent::ptr event);
    private:

    };
    //日志输出地
    class LogAppend
    {
    public:
        using ptr=std::shared_ptr<LogAppend>;
        virtual ~LogAppend();
        virtual void log(LogLevel::Level level,LogEvent::ptr event);
    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_format;
    };
    //日志器
    class Logger
    {
    public:
        Logger(const std::string& name="root");
        void log(LogLevel::Level level,LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warning(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppend(LogAppend::ptr append);
        void delAppend(LogAppend::ptr append);
        void set_level(LogLevel::Level level){m_level=level;}
        LogLevel::Level get_level()const {return m_level;}
    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::list<LogAppend::ptr> m_appends;
    };
    class Stdout_LogAppend : public LogAppend
    {
    public:
        using ptr=std::shared_ptr<Stdout_LogAppend>;
        virtual void log(LogLevel::Level levle,LogEvent::ptr event) override;
    };
    class Fileout_LogAppend : public LogAppend
    {
    public:
        using ptr=std::shared_ptr<Fileout_LogAppend>;
        Fileout_LogAppend(const std::string& filename);
        virtual void log(LogLevel::Level level,LogEvent::ptr evnet) override;
        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_file; 
    };
    


    Logger::Logger(const std::string& name="root"):m_name(name)
    {}
    void Logger::log(LogLevel::Level level,LogEvent::ptr event)
    {
        if(level>=m_level)
        {
            for(auto& append:m_appends)
            {
                append->log(level,event);
            }
        }
    }
    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::Level::DEBUG,event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::Level::INFO,event);
    }
    void Logger::warning(LogEvent::ptr event)
    {
        log(LogLevel::Level::WARNING,event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::Level::ERROR,event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::Level::FATAL,event);
    }
    void Logger::addAppend(LogAppend::ptr append)
    {
        m_appends.emplace_back(append);
    }
    void Logger::delAppend(LogAppend::ptr append)
    {
        auto it=m_appends.begin();
        while(it!=m_appends.end())
        {
            if(*it=append)
            {
                it=m_appends.erase(it);
            }
        }
    }
    void Stdout_LogAppend::log(LogLevel::Level level,LogEvent::ptr event)
    {
        if(level>=m_level)
        {
            std::cout<<m_format->format(event);
        }
    }
    Fileout_LogAppend::Fileout_LogAppend(const std::string& filename):m_filename(filename)
    {
        m_file.open(filename);
    }
    void Fileout_LogAppend::log(LogLevel::Level level,LogEvent::ptr event)
    {
        if(level>=m_level)
        {
            m_file<<m_format->format(event);
        }
    }
    bool Fileout_LogAppend::reopen()
    {
        if(m_file.is_open())
        {
            m_file.close();
        }
        m_file.open(m_filename);
        return m_file.is_open();
    }
    
}


