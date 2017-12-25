/*
 * =====================================================================================
 *
 *       Filename: cachequeue.hpp
 *        Created: 02/25/2016 01:01:40
 *  Last Modified: 12/24/2017 20:48:52
 *
 *    Description: fixed size linear cache queue
 *                 may use Boost.Circular Buffer in furture
 *
 *                 1. support push/pop at begin and end
 *                    pop an empty cache queue will do nothing
 *                    push to a full cache queue would override the data
 *                        a. void PushHead(T &&)/PopHead()
 *                        b. void PushBack(T &&)/PopBack()
 *
 *                 2. support access the first and last element
 *                    undefined behavior if current cache queue is zero-capacity/zero-length
 *                        a. T& Head()
 *                        b. T& Back()
 *
 *                 3. support circular iterator with self-increment
 *                    itreator do circular addition with current cache queue size
 *                        a. Begin()        // as v.begin()
 *
 *                    won't suppot end(), since p++ would never reach end()
 *                    instead we use static method Null(), usage of iteration:
 *
 *                         // for(int nIndex = 0; nIndex < stQ.Length(); ++nIndex){
 *                         //     func(stQ[nIndex]);
 *                         // }
 *
 *                         // auto p = stQ.Begin()
 *                         // for(int nIndex = 0; nIndex < stQ.Length(); ++nIndex, ++p){
 *                         //     func(*p);
 *                         // }
 *
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
#include "condcheck.hpp"

template<typename T, size_t N> class CacheQueue final
{
    public:

        // iterator can be null or invalid after construction
        // but when de-ref it we will do DerefCheck() and crash if dangling

        class Iterator final
        {
            private:
                CacheQueue<T, N> *m_CacheQueue;

            private:
                size_t m_At;

            public:
                Iterator()
                    : m_CacheQueue(nullptr)
                    , m_At(0)
                {}

               ~Iterator() = default;

            private:

                // hide from user
                // only CacheQueue::Begin() can construct it

                friend class CacheQueue<T, N>;
                Iterator(CacheQueue<T, N> *pCacheQueue = nullptr, size_t nAt = 0)
                    : m_CacheQueue(pCacheQueue)
                    , m_At(nAt)
                {}

            public:
                operator bool () const
                {
                    return m_CacheQueue != nullptr;
                }

            public:
                size_t At() const
                {
                    return m_At;
                }

            private:
                void DerefCheck() const
                {
                    condcheck(true
                            &&  m_CacheQueue
                            &&  m_CacheQueue->Capacity() > 0
                            && !m_CacheQueue->Empty()

                            && m_At >= 0
                            && m_At <  m_CacheQueue->Length());
                }

            private:
                void Forward(int nForward)
                {
                    if(nForward){
                        DerefCheck();
                        m_At = (m_At + nForward + m_CacheQueue->Length()) % m_CacheQueue->Length();
                    }
                }

            public:
                Iterator & operator ++ ()
                {
                    Forward(1);
                    return *this;
                }

                Iterator operator ++ (int)
                {
                    auto stRet = *this;
                    Forward(1);

                    return stRet;
                }

                Iterator operator + (int nForward)
                {
                    auto stRet = *this;
                    stRet.Forward(nForward);
                    return stRet;
                }

                T & operator * ()
                {
                    DerefCheck();
                    return m_CacheQueue->At(At());
                }

                const T & operator * () const
                {
                    DerefCheck();
                    return m_CacheQueue->At(At());
                }

                T * operator -> ()
                {
                    DerefCheck();
                    return &(m_CacheQueue->At(At()));
                }

                const T * operator -> () const
                {
                    DerefCheck();
                    return &(m_CacheQueue->At(At()));
                }

            public:
                bool operator == (const Iterator &stLHS) const
                {
                    return m_CacheQueue == stLHS.m_CacheQueue && m_At == stLHS.m_At;
                }

            public:
                Iterator & operator = (const Iterator &rstLHS)
                {
                    // simple class
                    // I won't use the swap here

                    m_CacheQueue = rstLHS.m_CacheQueue;
                    m_At         = rstLHS.m_At;
                }
        };

        // const version of the iterator
        // used for iteratioin on const cache queue

        class CIterator
        {
            private:
                CacheQueue<T, N> *m_CacheQueue;

            private:
                size_t m_At;

            public:
                CIterator()
                    : m_CacheQueue(nullptr)
                    , m_At(0)
                {}

               ~CIterator() = default;

            private:

                // hide it from user
                // only CacheQueue::Begin() can construct it

                friend class CacheQueue<T, N>;
                CIterator(CacheQueue<T, N> *pCacheQueue = nullptr, size_t nAt = 0)
                    : m_CacheQueue(pCacheQueue)
                    , m_At(nAt)
                {}

            public:
                operator bool () const
                {
                    return m_CacheQueue != nullptr;
                }

            public:
                size_t At() const
                {
                    return m_At;
                }

            private:
                void DerefCheck() const
                {
                    condcheck(false
                            &&  m_CacheQueue
                            &&  m_CacheQueue->Capacity() != 0
                            && !m_CacheQueue->Empty()

                            && m_At >= 0
                            && m_At <  m_CacheQueue->Length());
                }

            protected:
                void Forward(int nForward)
                {
                    if(nForward){
                        DerefCheck();
                        m_At = (m_At + nForward + m_CacheQueue->Length()) % m_CacheQueue->Length();
                    }
                }

            public:
                CIterator & operator ++ ()
                {
                    Forward(1);
                    return *this;
                }

                CIterator operator ++ (int)
                {
                    auto stRet = *this;
                    Forward(1);

                    return stRet;
                }

                CIterator operator + (int nForward)
                {
                    auto stRet = *this;
                    stRet.Forward(nForward);
                    return stRet;
                }

                const T & operator * () const
                {
                    DerefCheck();
                    return m_CacheQueue->At(At());
                }

                const T * operator -> () const
                {
                    DerefCheck();
                    return &(m_CacheQueue->At(At()));
                }

            public:
                bool operator == (const Iterator &stLHS) const
                {
                    return m_CacheQueue == stLHS.m_CacheQueue && m_At == stLHS.m_At;
                }

            public:
                Iterator & operator = (const Iterator &rstLHS)
                {
                    // simple class
                    // I won't use the swap here

                    m_CacheQueue = rstLHS.m_CacheQueue;
                    m_At         = rstLHS.m_At;
                }
        };

    private:
        std::array<T, N> m_CircleQ;

    private:
        size_t m_Head;          // point to the first element
        size_t m_CurrSize;      // how many elements we have in the queue

    public:
        CacheQueue()
            : m_CircleQ()
            , m_Head(0)
            , m_CurrSize(0)
        {}

       ~CacheQueue() = default;

    public:
        static Iterator Null()
        {
            return {nullptr, 0};
        }

        static CIterator CNull()
        {
            return {nullptr, 0};
        }

    public:
        Iterator Begin()
        {
            return Empty() ? Null() : Iterator(this, 0);
        }

        CIterator Begin() const
        {
            return Empty() ? Null() : CIterator(this, 0);
        }

        CIterator CBegin() const
        {
            return Empty() ? Null() : CIterator(this, 0);
        }

    public:
        constexpr size_t Capacity() const
        {
            return m_CircleQ.size();
        }

    public:
        size_t Size() const
        {
            return m_CurrSize;
        }

        size_t Length() const
        {
            return Size();
        }

    public:
        bool Empty() const
        {
            return m_CurrSize == 0;
        }

        bool Full() const
        {
            return m_CurrSize == Capacity();
        }

    public:
        T &Head()
        {
            condcheck((Capacity() != 0) && (!Empty()));
            return m_CircleQ[m_Head];
        }

        const T &Head() const
        {
            condcheck((Capacity() != 0) && (!Empty()));
            return m_CircleQ[m_Head];
        }

        T &Back()
        {
            condcheck((Capacity() != 0) && (!Empty()));
            return m_CircleQ[(m_Head + Length() + Capacity() - 1) % Capacity()];
        }

        const T &Back() const
        {
            condcheck((Capacity() != 0) && (!Empty()));
            return m_CircleQ[(m_Head + Length() + Capacity() - 1) % Capacity()];
        }

    public:
        template<typename... U> void PushHead(U&&... u)
        {
            condcheck(Capacity() != 0);
            if(Empty()){
                m_Head       = 0;
                m_CircleQ[0] = T(std::forward<U>(u)...);
                m_CurrSize   = 1;
            }else{
                m_Head = ((m_Head + Capacity() - 1) % Capacity());
                m_CircleQ[m_Head] = T(std::forward<U>(u)...);
                m_CurrSize = std::min<size_t>(m_CurrSize + 1, Capacity());
            }
        }

        template<typename... U> void PushBack(U&&... u)
        {
            condcheck(Capacity() != 0);
            if(Empty()){
                m_Head       = 0;
                m_CircleQ[0] = T(std::forward<U>(u)...);
                m_CurrSize   = 1;
            }else if(Full()){
                m_CircleQ[m_Head] = T(std::forward<U>(u)...);
                m_Head = (m_Head + 1) % Capacity();
            }else{
                m_CircleQ[(m_Head + Length()) % Capacity()] = T(std::forward<U>(u)...);
                m_CurrSize = std::min<size_t>(m_CurrSize + 1, Capacity());
            }
        }

    public:
        void PopHead()
        {
            if(!Empty()){
                m_Head = (m_Head + 1) % Capacity();
                m_CurrSize--;
            }
        }

        void PopBack()
        {
            if(!Empty()){
                m_CurrSize--;
            }
        }

    public:
        void Clear()
        {
            m_Head = 0;
            m_CurrSize = 0;
        }

    public:
        void Reset()
        {
            // circular queue rotation
            // need to optimize since we do extra copy when queue is not full
            std::rotate(m_CircleQ.begin(), m_CircleQ.end(), m_CircleQ.begin() + m_Head);
        }

    public:
        T &Off(size_t nOff)
        {
            return m_CircleQ[nOff];
        }

        const T &Off(size_t nOff) const
        {
            return m_CircleQ[nOff];
        }
        
    public:
        T &At(size_t nIndex)
        {
            return m_CircleQ[(m_Head + nIndex + Capacity()) % Capacity()];
        }

        const T &At(size_t nIndex) const
        {
            return m_CircleQ[(m_Head + nIndex + Capacity()) % Capacity()];
        }

    public:
        T & operator [] (size_t nIndex)
        {
            return At(nIndex);
        }

        const T & operator [] (size_t nIndex) const
        {
            return At(nIndex);
        }

    public:
        size_t HeadOff() const
        {
            return m_Head;
        }

        size_t BackOff() const
        {
            return (m_Head + Length() - 1 + Capacity()) % Capacity();
        }
};
