#pragma once

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

namespace MindbniM
{

    class Buffer
    {
    public:
        // 构造函数：初始化缓冲区，默认大小为 1024 字节
        Buffer(int Size = 1024):_buffer(Size),_rPos(0),_wPos(0)
        {}

        ~Buffer() = default;

        // 返回当前可写字节数
        size_t Write_ableBytes() const {return _buffer.size()-_wPos;}

        // 返回当前可读字节数
        size_t Read_ableBytes() const{ return _buffer.size()-_rPos;}

        // 返回当前可插入字节数（即读位置之前的空间）
        size_t Insert_ableBytes() const{return _rPos;}

        // 返回指向可读数据开始位置的常量指针
        const char *Peek() const{ return Begin_Ptr()+_rPos;}

        // 确保缓冲区有足够的可写空间，如果不足则调整缓冲区大小
        void Ensure_Writeable(size_t len);

        // 更新缓冲区的写位置，表示 `len` 字节的数据已被写入
        void ReWritten(size_t len);

        // 更新读位置, 表示已经读完len字节
        void Retrieve(size_t len);

        // 更新读位置, 表示已经读到end
        void Retrieve_Ptr(const char *end);

        // 清空缓冲区，重置读写位置
        void Retrieve_All();

        // 将所有可读数据转换为字符串，并清空缓冲区
        std::string Retrieve_AllToStr();

        // 返回指向可写区域开始位置的常量指针
        const char *Begin_Write() const{return Begin_Ptr()+_wPos;}

        // 返回指向可写区域开始位置的指针
        char *Begin_Write(){return Begin_Ptr()+_wPos;}

        // 将字符串追加到缓冲区
        void Append(const std::string &str);

        // 将C风格字符串追加到缓冲区
        void Append(const char *str, size_t len);

        // 将原始数据追加到缓冲区
        void Append(const void *data, size_t len);

        // 将另一个缓冲区的数据追加到当前缓冲区
        void Append(const Buffer &buff);

        // 从文件描述符 `fd` 读取数据到缓冲区
        ssize_t ReadFd(int fd, int *Errno);

        // 将缓冲区的数据写入到文件描述符 `fd`
        ssize_t WriteFd(int fd, int *Errno);

    private:
        // 返回缓冲区起始位置的非const版本指针
        char *Begin_Ptr(){return &(*_buffer.begin());}

        // 返回缓冲区起始位置的常量指针

        const char *Begin_Ptr() const{return &(*_buffer.begin());}

        // 调整缓冲区, 如果空间不足就扩容, 如果空间足够就调整数据到开头位置
        void MakeSpace_(size_t len);

        std::vector<char> _buffer; 
        std::atomic<std::size_t> _rPos;
        std::atomic<std::size_t> _wPos; 
    };
    void Buffer::MakeSpace_(size_t len)
    {
        if(Write_ableBytes()+Insert_ableBytes()<len)
        {
            _buffer.resize(_wPos+len+1);
        }
        else
        {
            int RAB=Read_ableBytes();
            std::copy(_buffer.begin()+_rPos,_buffer.begin()+_wPos,_buffer.begin());
            _rPos=0;
            _wPos=RAB;
            assert(RAB==Read_ableBytes());
        }
    }
    void Buffer::Ensure_Writeable(size_t len)
    {
        if(_buffer.size()-_wPos<len)
        {
            MakeSpace_(len);
        }
        assert(Write_ableBytes()>=len);
    }
    void Buffer::ReWritten(size_t len)
    {
        assert(_wPos+len<=_buffer.size());
        _wPos+=len;
    }
    void Buffer::Retrieve(size_t len)
    {
        assert(_rPos+len<=Read_ableBytes());
        _rPos+=len;
    }
    void Buffer::Retrieve_Ptr(const char *end)
    {
        assert(end>=Peek());
        Retrieve(end-Peek());
    }
    void Buffer::Retrieve_All()
    {
        std::fill(_buffer.begin(), _buffer.end(), 0);
        _wPos=0;
        _rPos=0;
    }
    std::string Buffer::Retrieve_AllToStr()
    {
        std::string str(Peek(),Read_ableBytes());
        Retrieve_All();
        return str;
    }
    void Buffer::Append(const std::string &str)
    {
        Append(str.c_str(),str.size());
    }
    void Buffer::Append(const char *str, size_t len)
    {
        assert(str);
        Ensure_Writeable(len);
        std::copy(str,str+len,Begin_Write());
        ReWritten(len);
    }
    void Buffer::Append(const void *data, size_t len)
    {
        assert(data);
        Append(static_cast<const char*>(data),len);
    }
    void Buffer::Append(const Buffer &buff)
    {
        Append(buff.Peek(),buff.Read_ableBytes());
    }
    ssize_t Buffer::ReadFd(int fd, int *Errno)
    {
        char buff[1024*64]={0};
        struct iovec io[2];
        int wsize=Write_ableBytes();
        io[0].iov_base=Begin_Write();
        io[0].iov_len=wsize;
        io[1].iov_base=buff;
        io[1].iov_len=sizeof(buff);
        ssize_t n=readv(fd,io,2);
        if(n<0)
        {
            *Errno=errno;
        }
        else if(static_cast<size_t>(n)<wsize)
        {
            _wPos+=n;
        }
        else 
        {
            _wPos=_buffer.size();
            Append(buff,n-wsize);
        }
        return n;
    }
    ssize_t Buffer::WriteFd(int fd, int *Errno)
    {
        ssize_t n=write(fd,Peek(),Read_ableBytes());
        if(n<0)
        {
            *Errno=errno;
            return n;
        }
        _rPos+=n;
        return n;
    }
}
