/*
 * =====================================================================================
 *
 *       Filename: cachequeue.hpp
 *        Created: 02/25/2016 01:01:40
 *  Last Modified: 04/11/2017 19:42:22
 *
 *    Description: linear cache queue
 *
 *                      1. no PushBack() since this is for LRU
 *                      2. accept N == 0 since which means we disable cache
 *                      3. to enable following traverse
 *
 *                          // for(stCQ.Reset(); !stCQ.Done(); stCQ.Forward()){
 *                          //     stCQ.Current();
 *                          // }
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */


#pragma once
#include <array>
#include <algorithm>

template<typename T, size_t N>
class CacheQueue final
{
    private:
        std::array<T, N> m_CircleQ;

    private:
        size_t m_Head;
        size_t m_Size;
        size_t m_CheckCount;

    public:
        CacheQueue()
            : m_CircleQ()
            , m_Head(0)
            , m_Size(0)
            , m_CheckCount(0){}

       ~CacheQueue() = default;

    public:
        size_t Capacity() const
        {
            return N;
        }

        size_t Size() const
        {
            return m_Size;
        }

        bool Empty() const
        {
            return m_Size == 0;
        }

        bool Full() const
        {
            return m_Size == N;
        }

        size_t Index() const
        {
            return (m_Head + m_CheckCount) % N;
        }

        T &Head()
        {
            // undefined when N == 0
            // but seems std::array<T, 0> won't fail
            // request of CacheQueue<T, N> is by std::array<T, N>
            return m_CircleQ[m_Head];
        }

        T &Back()
        {
            return m_CircleQ[(m_Head + m_Size - 1 + N) % N];
        }

        T &Current()
        {
            return m_CircleQ[Index()];
        }

        void SwapHead(size_t nIndex)
        {
            std::swap(Head(), m_CircleQ[nIndex]);
        }

        template<typename... U> void PushHead(U&&... u)
        {
            // 1. don't call it during the traversing
            // 2. may throw
            if(m_Size == 0){
                // empty queue we need to use PushBack() instead
                m_CircleQ[0] = T(std::forward<U>(u)...);
                m_Head       = 0;
                m_Size       = 1;
            }else{
                m_CircleQ[(m_Head - 1 + N) % N] = T(std::forward<U>(u)...);
                m_Head = ((m_Head - 1 + N) % N);
                m_Size = std::min(N, m_Size + 1);
            }
        }

        // TODO
        // for LRU we don't need this pop function
        // keep it for completion
        void PopHead()
        {
            // 1. if there is no element, do nothing
            if(Empty()){ return; }

            // now at least there is one element

            // 2. move the head forward
            m_Head = ((m_Head + 1) % N);

            // 3. decrease the size by one
            m_Size--;
        }

        void Reset()
        {
            m_CheckCount = 0;
        }

        bool Done()
        {
            return m_CheckCount == m_Size;
        }

        void Forward()
        {
            m_CheckCount++;
        }

        void Clear()
        {
            m_Head       = 0;
            m_Size       = 0;
            m_CheckCount = 0;
        }
};
