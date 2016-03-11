/*
 * =====================================================================================
 *
 *       Filename: cachequeue.hpp
 *        Created: 02/25/2016 01:01:40
 *  Last Modified: 03/10/2016 23:52:54
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
    public:
        CacheQueue()
            : m_Head(0)
            , m_Size(0)
            , m_CheckCount(0){}

       ~CacheQueue() = default;

    public:
        bool Size()
        {
            return m_Size;
        }

        bool Full()
        {
            return m_Size == N;
        }

        T &Head()
        {
            // TODO
            // undefined when N == 0
            return m_CircleQ[m_Head];
        }

        T &Back()
        {
            // TODO
            // undefined when N == 0, but this compiles without warnings
            return m_CircleQ[(m_Head + m_Size - 1 + N) % N];
        }

        T &Current()
        {
            return m_CircleQ[(m_Head + m_CheckCount) % N];
        }

        size_t Index()
        {
            return (m_Head + m_CheckCount) % N;
        }

        void SwapHead(size_t nIndex)
        {
            // won't check parameters
            // index is absolute index
            std::swap(m_CircleQ[m_Head], m_CircleQ[nIndex]);
        }

        void PushHead(T stT)
        {
            // 1. don't call it during the traversing
            // 2. may throw
            if(m_Size == 0){
                // empty queue we need to use PushBack() instead
                m_CircleQ[0] = stT;
                m_Head       = 0;
                m_Size       = 1;
            }else{
                m_CircleQ[(m_Head - 1 + N) % N] = stT;
                m_Head = ((m_Head - 1 + N) % N);
                m_Size = std::min(N, m_Size + 1);
            }
        }

        void Reset()
        {
            // for traversal
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
            // get rid of all elements
            m_Head       = 0;
            m_Size       = 0;
            m_Current    = 0;
            m_CheckCount = 0;
        }

    private:
        std::array<T, N> m_CircleQ;
        int              m_Head;
        size_t           m_Size;
        int              m_Current;
        int              m_CheckCount;
};
