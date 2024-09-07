#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <functional>
namespace MindbniM
{
    class Logger;
    // 日志事件
    class LogEvent
    {
    public:
        using ptr = std::shared_ptr<LogEvent>;
        LogEvent();

        std::string _file;               // 文件名
        int _line;                       // 行号
        uint32_t _elapse;                // 程序启动的毫秒数
        uint32_t _threadId;              // 线程id
        uint32_t _fiberId;               // 协程id
        uint64_t _time;                  // 时间戳
        std::string _message;            // 消息
        std::shared_ptr<Logger> _logger; // 所属日志器
    };
    // 日志等级
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARNING = 3,
            ERROR = 4,
            FATAL = 5
        };
        // 输出对应的字符串
        static std::string to_string(LogLevel::Level level)
        {
            switch (level)
            {
#define XX(name)      \
    case Level::name: \
        return #name; \
        break;
                XX(DEBUG)
                XX(INFO)
                XX(WARNING)
                XX(ERROR)
                XX(FATAL)
            default:
                return "UNKNOW";
#undef XX
            }
            return "UNKNOW";
        }
    };
    // 日志格式器

    // 格式："%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    //*  %m 消息
    //*  %p 日志级别
    //*  %r 累计毫秒数
    //*  %c 日志名称
    //*  %t 线程id
    //*  %n 换行
    //*  %d 时间
    //*  %f 文件名
    //*  %l 行号
    //*  %T 制表符
    //*  %F 协程id

    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        // 对给出的日志格式初始化m_items
        LogFormatter(const std::string &formatstr) : m_format(formatstr) { init(); }
        // 上层总解析
        std::string format(LogLevel::Level level, LogEvent::ptr event);
        void init();
        // 日志解析方法
        class FormatItem
        {
        public:
            using ptr = std::shared_ptr<FormatItem>;
            virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) = 0;
            virtual ~FormatItem() {}
        };

    private:
        std::string m_format;                 // 格式
        std::vector<FormatItem::ptr> m_items; // 日志格式解析后需要的方法
        bool m_error = false;                 // 日志格式是否错误
    };
    // 日志输出地
    class LogAppend
    {
    public:
        using ptr = std::shared_ptr<LogAppend>;
        virtual ~LogAppend();
        virtual void log(LogLevel::Level level, LogEvent::ptr event);

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_format;
    };
    // 日志器
    class Logger
    {
    public:
        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warning(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppend(LogAppend::ptr append);
        void delAppend(LogAppend::ptr append);
        void set_level(LogLevel::Level level) { m_level = level; }
        LogLevel::Level get_level() const { return m_level; }
        std::string get_name() { return m_name; }

    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::list<LogAppend::ptr> m_appends;
    };

    class Stdout_LogAppend : public LogAppend
    {
    public:
        using ptr = std::shared_ptr<Stdout_LogAppend>;
        virtual void log(LogLevel::Level levle, LogEvent::ptr event) override;
    };
    class Fileout_LogAppend : public LogAppend
    {
    public:
        using ptr = std::shared_ptr<Fileout_LogAppend>;
        Fileout_LogAppend(const std::string &filename);
        virtual void log(LogLevel::Level level, LogEvent::ptr evnet) override;
        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_file;
    };

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_message;
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::to_string(level);
        }
    };
    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_line;
        }
    };
    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_file;
        }
    };
    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_elapse;
        }
    };
    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_threadId;
        }
    };
    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_fiberId;
        }
    };
    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->_logger->get_name();
        }
    };
    class DateFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateFormatItem(const std::string format = "%Y-%m-%d %H:%M:%S") : m_format(format) {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
        }

    private:
        std::string m_format;
    };
    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }

    private:
    };
    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }

    private:
    };
    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str = "") : m_str(str) {}
        virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_str;
        }

    private:
        std::string m_str;
    };

    Logger::Logger(const std::string &name = "root") : m_name(name)
    {
    }
    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            for (auto &append : m_appends)
            {
                append->log(level, event);
            }
        }
    }
    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::Level::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::Level::INFO, event);
    }
    void Logger::warning(LogEvent::ptr event)
    {
        log(LogLevel::Level::WARNING, event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::Level::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::Level::FATAL, event);
    }
    void Logger::addAppend(LogAppend::ptr append)
    {
        m_appends.emplace_back(append);
    }
    void Logger::delAppend(LogAppend::ptr append)
    {
        auto it = m_appends.begin();
        while (it != m_appends.end())
        {
            if (*it = append)
            {
                it = m_appends.erase(it);
            }
        }
    }
    void Stdout_LogAppend::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << m_format->format(level, event);
        }
    }
    Fileout_LogAppend::Fileout_LogAppend(const std::string &filename) : m_filename(filename)
    {
        m_file.open(filename);
    }
    void Fileout_LogAppend::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_file << m_format->format(level, event);
        }
    }
    bool Fileout_LogAppend::reopen()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
        m_file.open(m_filename);
        return m_file.is_open();
    }

    std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event)
    {
        std::ostringstream os;
        for (auto &fmat : m_items)
        {
            fmat->format(os, level, event);
        }
        return os.str();
    }
    void LogFormatter::init()
    {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (int i = 0; i < m_format.size(); ++i)
        {
            if (m_format[i] != '%')
            {
                nstr.push_back(m_format[i]);
                continue;
            }
            if ((i + 1 < m_format.size()) && m_format[i + 1] == '%')
            {
                nstr.push_back('%');
                continue;
            }
            size_t n = i + 1;
            std::string str;
            std::string fmt;
            int flag = 0;
            int flag_begin = 0;
            while (n < m_format.size())
            {
                if (flag == 0 && !isalpha(m_format[i]) && m_format[i] != '{' && m_format[i] != '}')
                {
                    str = m_format.substr(i + 1, n - i - 1);
                    break;
                }
                if (flag == 0)
                {
                    if (m_format[n] == '{')
                    {
                        str = m_format.substr(i + 1, n - i - 1);
                        flag = 1;
                        flag_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (flag == 1)
                {
                    if (m_format[n] == '}')
                    {
                        fmt = m_format.substr(flag_begin + 1, n - flag_begin - 1);
                        flag = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_format.size())
                {
                    if (str.empty())
                    {
                        str = m_format.substr(i + 1);
                    }
                }
            }
            if (flag == 0)
            {
                if (!nstr.empty())
                {
                    vec.emplace_back(nstr, "", 0);
                    nstr.clear();
                }
                vec.emplace_back(str, fmt, 1);
                i = n - 1;
            }
            else if (flag == 1)
            {
                std::cout << "日志格式错误: " << m_format << " - " << m_format.substr(i) << std::endl;
                m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }
        if (!nstr.empty())
        {
            vec.emplace_back(nstr, "", 0);
        }
        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_hash = {
#define XX(str, c) {#str, [](const std::string &fmt) { return FormatItem::ptr(new c(fmt)); }}
            XX(m, MessageFormatItem),  // m:消息
            XX(p, LevelFormatItem),    // p:日志级别
            XX(r, ElapseFormatItem),   // r:累计毫秒数
            XX(c, NameFormatItem),     // c:日志名称
            XX(t, ThreadIdFormatItem), // t:线程id
            XX(d, DateFormatItem),     // d:时间
            XX(f, FilenameFormatItem), // f:文件名
            XX(l, LineFormatItem),     // l:行号
            XX(T, TabFormatItem),      // T:Tab
            XX(F, FiberIdFormatItem),  // F:协程id
            XX(n, NewLineFormatItem),  // n:换行
#undef XX
        };
        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_hash.find(std::get<0>(i));
                if (it == s_format_hash.end())
                { // 若没有找到则用StringFormatItem显示错误信息 并设置错误标志位
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                { // 返回相应格式的FormatItem，其中std::get<1>(i)作为cb的参数
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
        }
    }

}
