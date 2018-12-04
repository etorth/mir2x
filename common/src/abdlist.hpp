/*
 * =====================================================================================
 *
 *       Filename: abdlist.hpp
 *        Created: 06/01/2018 14:17:59
 *    Description: array based double linked list
 *                 only use ValueType with trivial constructor/destructor
 *
 *                 ABDList is not a container
 *                 it won't call destructor when you pop an element
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

#ifndef __ABLIST_HPP__
#define __ABLIST_HPP__

#include <array>
#include <vector>
#include <fstream>
#include <iostream>

template<typename ValueType, size_t SBufLen = 0> class ABDList
{
    private:
        static constexpr auto NPos = (size_t)(-1);

    private:
        struct ABDListNode
        {
            size_t    Prev;
            size_t    Next;
            ValueType Value;
        };

    private:
        std::vector<ABDListNode> m_DBuf;

    private:
        std::array<ABDListNode, SBufLen> m_SBuf;

    private:
        size_t m_Used0;
        size_t m_Used1;
        size_t m_Free0;
        size_t m_Free1;

    public:
        ABDList(size_t nCap = 0)
            : m_DBuf()
            , m_SBuf()
            , m_Used0(NPos)
            , m_Used1(NPos)
            , m_Free0(0)
            , m_Free1(0)
        {
            static_assert(std::is_pod<ValueType>::value, "ABDList only supports POD data");
            if(nCap > m_SBuf.size()){
                m_DBuf.resize(nCap - m_SBuf.size());
            }
            Reset();
        }

    public:
        size_t Capacity() const
        {
            return m_SBuf.size() + m_DBuf.size();
        }

    public:
        ValueType &Head()
        {
            return CurrNode(m_Used0).Value;
        }

        const ValueType &Head() const
        {
            return CurrNode(m_Used0).Value;
        }

        ValueType &Back()
        {
            return CurrNode(m_Used1).Value;
        }

        const ValueType &Back() const
        {
            return CurrNode(m_Used1).Value;
        }

    public:
        size_t Begin() const
        {
            return m_Used0;
        }

        size_t End() const
        {
            return (m_Used1 < Capacity()) ? Next(m_Used1) : NPos;
        }

        size_t HeadOff() const
        {
            return m_Used0;
        }

        size_t BackOff() const
        {
            return m_Used1;
        }

    public:
        ValueType & operator [] (size_t nPos)
        {
            return CurrNode(nPos).Value;
        }

        const ValueType & operator [] (size_t nPos) const
        {
            return CurrNode(nPos).Value;
        }

    public:
        size_t Prev(size_t nPos) const
        {
            return CurrNode(nPos).Prev;
        }

        size_t Next(size_t nPos) const
        {
            return CurrNode(nPos).Next;
        }

    private:
        ABDListNode &PrevNode(size_t nPos)
        {
            return CurrNode(CurrNode(nPos).Prev);
        }

        ABDListNode &NextNode(size_t nPos)
        {
            return CurrNode(CurrNode(nPos).Next);
        }

        ABDListNode &CurrNode(size_t nPos)
        {
            if(nPos < m_SBuf.size()){
                return m_SBuf[nPos];
            }
            return m_DBuf[nPos - m_SBuf.size()];
        }

    private:
        const ABDListNode &PrevNode(size_t nPos) const
        {
            return CurrNode(CurrNode(nPos).Prev);
        }

        const ABDListNode &NextNode(size_t nPos) const
        {
            return CurrNode(CurrNode(nPos).Next);
        }

        const ABDListNode &CurrNode(size_t nPos) const
        {
            if(nPos < m_SBuf.size()){
                return m_SBuf[nPos];
            }
            return m_DBuf[nPos - m_SBuf.size()];
        }

    public:
        // assume nPos is valid in data list
        // now move it as the new used0 but keeps all others the same
        void MoveHead(size_t nPos)
        {
            if(nPos == m_Used0){
                return;
            }

            auto nPosNext = Next(nPos);
            auto nPosPrev = Prev(nPos);

            // we know nPos in data list and not the head
            // then it should have a prev node

            CurrNode(nPosPrev).Next = nPosNext;
            if(nPosNext < Capacity()){
                CurrNode(nPosNext).Prev = nPosPrev;
            }

            CurrNode(nPos   ).Prev = NPos;
            CurrNode(nPos   ).Next = m_Used0;
            CurrNode(m_Used0).Prev = nPos;

            // shouldn't touch free0/1
            // change happens in data list only

            m_Used0 = nPos;
            if(m_Used1 == nPos){
                m_Used1 = nPosPrev;
            }
        }

        // assume nPos is valid in data list
        // now move it as the new used1 but keeps all others the same
        void MoveBack(size_t nPos)
        {
            if(nPos == m_Used1){
                return;
            }

            // we know nPos in data list and not the back
            // then it should have a next node

            auto nPosPrev = Prev(nPos);
            auto nPosNext = Next(nPos);

            if(nPosPrev < Capacity()){
                CurrNode(nPosPrev).Next = nPosNext;
                CurrNode(nPosNext).Prev = nPosPrev;
            }else{
                CurrNode(nPosNext).Prev = NPos;
            }

            CurrNode(nPos).Prev = m_Used1;
            CurrNode(nPos).Next = m_Free0;

            CurrNode(m_Used1).Next = nPos;
            if(m_Free0 < Capacity()){
                CurrNode(m_Free0).Prev = nPos;
            }

            // shouldn't touch free0/1
            // change happens in data list only

            if(m_Used0 == nPos){
                m_Used0 = nPosNext;
            }
            m_Used1 = nPos;
        }

    public:
        // pick the free1 node
        // and make it immediately before the specified node
        size_t Insert(size_t nPos, ValueType &&stValue)
        {
            if(FreeNodeEmpty()){
                AppendFreeNode(1);
            }

            auto nFreeNode = m_Free1;
            CurrNode(nFreeNode).Value = std::move(stValue);

            PickFree1Node();

            CurrNode(nFreeNode).Prev = Prev(nPos);
            CurrNode(nFreeNode).Next = nPos;

            // link the removed node to requested position
            CurrNode(nPos).Prev = nFreeNode;
            if(Prev(nPos) < Capacity()){
                PrevNode(nPos).Next = nFreeNode;
            }

            return nFreeNode;
        }

    public:
        // pick the specified node
        // and make is as the new free1 node
        // don't use it to implement pophead/back
        size_t Erase(size_t nPos)
        {
            // if at begin/end need to reset used0/1 and free0/1
            // better use the specified functions directly

            if(nPos == m_Used0){
                PopHead();
                return Begin();
            }

            if(nPos == m_Used1){
                PopBack();
                return End();
            }

            // not begin not end
            // in the middle, so we have prev/next well defined

            auto nPosNext = Next(nPos);
            PrevNode(nPos).Next = nPosNext;
            NextNode(nPos).Prev = Prev(nPos);

            PushFree1Node(nPos);
            return nPosNext;
        }

    public:
        // pick the free1 node
        // and make is as the new used0 node
        template<typename... U> void PushHead(U&&... u)
        {
            if(FreeNodeEmpty()){
                AppendFreeNode(1);
            }

            auto nFreeNode = m_Free1;
            CurrNode(nFreeNode).Value = ValueType(std::forward<U>(u)...);

            PickFree1Node();
            CurrNode(nFreeNode).Prev = NPos;

            if(Empty()){
                CurrNode(nFreeNode).Next = m_Free0;
                if(m_Free0 < Capacity()){
                    CurrNode(m_Free0).Prev = nFreeNode;
                }
                m_Used0 = nFreeNode;
                m_Used1 = nFreeNode;
            }else{
                CurrNode(nFreeNode).Next = m_Used0;
                CurrNode(m_Used0  ).Prev = nFreeNode;
                m_Used0 = nFreeNode;
            }
        }

        // pick the free0 node
        // and make is as the new used1 node
        template<typename... U> void PushBack(U&&... u)
        {
            if(FreeNodeEmpty()){
                AppendFreeNode(1);
            }

            CurrNode(m_Free0).Value = ValueType(std::forward<U>(u)...);
            if(Empty()){
                m_Used0 = m_Free0;
            }
            m_Used1 = m_Free0;

            if(m_Free0 == m_Free1){
                m_Free0 = Capacity();
                m_Free1 = m_Free0;
            }else{
                m_Free0 = Next(m_Free0);
            }
        }

    public:
        // pop the list head node
        // and make is as the new free1 node
        void PopHead()
        {
            auto nOldHead = m_Used0;
            if(m_Used0 == m_Used1){
                m_Used0 = NPos;
                m_Used1 = NPos;
            }else{
                m_Used0 = Next(m_Used0);
                CurrNode(m_Used0).Prev = NPos;
            }
            PushFree1Node(nOldHead);
        }

        // pop the list back node
        // and make it as the new free0 node
        void PopBack()
        {
            if(FreeNodeEmpty()){
                m_Free0 = m_Used1;
                m_Free1 = m_Free0;
            }else{
                m_Free0 = m_Used1;
            }

            // we have only one node
            // need to reset used0 and used1 as npos
            if(m_Used0 == m_Used1){
                m_Used0 = NPos;
                m_Used1 = NPos;
            }else{
                m_Used1 = Prev(m_Used1);
            }
        }

    public:
        bool Empty() const
        {
            return m_Used0 == NPos;
        }

        size_t Length() const
        {
            size_t nCount = 0;
            for(size_t nIndex = Begin(); nIndex != End(); nIndex = Next(nIndex)){
                nCount++;
            }
            return nCount;
        }

        void Clear()
        {
            Reset();
        }

    private:
        bool FreeNodeEmpty() const
        {
            return m_Free0 >= Capacity();
        }

    private:
        // assume free1 is valid
        // remove free1 from the free node list and relink the free list
        void PickFree1Node()
        {
            if(Prev(m_Free1) < Capacity()){
                PrevNode(m_Free1).Next = Capacity();
            }

            // we assume free1 is valid
            // then if free0 == free1 there must be only one free node
            if(m_Free0 == m_Free1){
                m_Free0 = Capacity();
                m_Free1 = m_Free0;
            }else{
                m_Free1 = Prev(m_Free1);
            }
        }

        // assume nPos is valid and has been removed from the data list
        // make nPos node as the the new free1 node and relink the free list
        void PushFree1Node(size_t nPos)
        {
            if(FreeNodeEmpty()){
                CurrNode(nPos).Prev = m_Used1;
                CurrNode(nPos).Next = Capacity();
                if(m_Used1 < Capacity()){
                    CurrNode(m_Used1).Next = nPos;
                }
                m_Free0 = nPos;
                m_Free1 = nPos;
            }else{
                CurrNode(nPos   ).Prev = m_Free1;
                CurrNode(nPos   ).Next = Capacity();
                CurrNode(m_Free1).Next = nPos;
                m_Free1 = nPos;
            }
        }

    private: 
        // only call this function if there is no free node
        // when there is no free node:
        //      1. Next(m_Used1) == Capacity()
        //      2. m_Free0       == Capacity()
        //      3. m_Free1       == Capacity()
        void AppendFreeNode(size_t nCnt)
        {
            auto nOldCap = Capacity();
            m_DBuf.resize(m_DBuf.size() + nCnt);

            // don't need to link new node to the whole list
            // since we know the Next of ``last node of the whole list" already is Capacity()

            CurrNode(nOldCap).Prev = m_Used1;
            CurrNode(nOldCap).Next = nOldCap + 1;

            for(size_t nIndex = nOldCap + 1; nIndex < Capacity(); ++nIndex){
                CurrNode(nIndex).Prev = nIndex - 1;
                CurrNode(nIndex).Next = nIndex + 1;
            }

            m_Free0 = nOldCap;
            m_Free1 = Capacity() - 1;
        }

    private:
        void Reset()
        {
            for(size_t nIndex = 0; nIndex < Capacity(); ++nIndex){
                CurrNode(nIndex).Prev = nIndex - 1;
                CurrNode(nIndex).Next = nIndex + 1;
            }

            m_Used0 = NPos;
            m_Used1 = NPos;

            m_Free0 = 0;
            m_Free1 = (Capacity() > 0) ? (Capacity() - 1) : 0;
        }

    public:
        void Debug(std::ostream &f = std::cout)
        {
            f << "Used0 : " << m_Used0 << std::endl;
            f << "Used1 : " << m_Used1 << std::endl;

            f << "Free0 : " << m_Free0 << std::endl;
            f << "Free1 : " << m_Free1 << std::endl;

            f << "Capacity : " << Capacity() << std::endl;

            if(!Empty()){
                for(size_t nIndex = Begin(), nLoc = 0; nIndex != End(); nIndex = Next(nIndex), ++nLoc){
                    f << "DataLoc = " << nLoc << ", Loc = " << nIndex << ", Prev = " << Prev(nIndex) << ", Next = " << Next(nIndex) << std::endl;
                }
            }

            if(!FreeNodeEmpty()){
                for(size_t nIndex = m_Free0, nLoc = 0; nIndex != Capacity(); nIndex = Next(nIndex), ++nLoc){
                    f << "FreeLoc = " << nLoc << ", Loc = " << nIndex << ", Prev = " << Prev(nIndex) << ", Next = " << Next(nIndex) << std::endl;
                }
            }
        }
};
#endif
