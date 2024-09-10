#pragma once
#include <vector>
#include <string>
#include <deque>
namespace MindbniM
{
    template <class T>
    class _Buffer
    {
    public:
        template<class V>
        friend std::ostream &operator<<(std::ostream &o, const _Buffer<V> &buff);
        _Buffer() = default;
        template <class ITERATOR>
        _Buffer(ITERATOR begin, ITERATOR end) : m_buff(begin, end)
        {}
        _Buffer(const _Buffer<T> &buff)
        {
            m_buff = buff.m_buff;
        }
        _Buffer(_Buffer<T> &&buff) : m_buff(std::move(buff))
        {
        }
        template <class ITERATOR>
        void push_back(ITERATOR begin, ITERATOR end)
        {
            m_buff.insert(m_buff.end(), begin, end);
        }
        void pop_front(int size)
        {
            if (size > m_buff.size())
            {
                m_buff.clear();
            }
            else
                m_buff.erase(m_buff.begin(), m_buff.begin() + size);
        }
        T &operator[](size_t pos)
        {
            return m_buff[pos];
        }
        void operator=(const _Buffer<T> &buff)
        {
            m_buff = buff.m_buff;
        }
        void operator=(_Buffer<T> &&buff)
        {
            m_buff = std::move(buff.m_buff);
        }
        const T &operator[](size_t pos) const
        {
            return m_buff[pos];
        }

    private:
        std::deque<T> m_buff;
    };
    template <class T>
    std::ostream &operator<<(std::ostream &o, const _Buffer<T> &buff)
    {
        for (auto &i : buff.m_buff)
        {
            o << i;
        }
        return o;
    }
    using Buffer = _Buffer<char>;
}