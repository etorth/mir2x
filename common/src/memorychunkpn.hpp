/*
 * =====================================================================================
 *
 *       Filename: memorychunkpn.hpp
 *        Created: 05/12/2016 23:01:23
 *  Last Modified: 05/18/2016 00:37:19
 *
 *    Description: fixed size memory block pool
 *                 simple implementation for performance
 *                 thread safe is optional, but self-contained
 *
 *
 *                 +---+   +---+   +---+ <----------------------------------
 *                 |   |   |   |   |   | <------ this is one buffer      ^
 *                 |   |   |   |   |   |                                 |
 *                 +---+   +---+   +---+  .....                        this is a memory unit
 *                 |   |   |   |   |   |                                 |
 *                 |   |   |   |   |   |                                 V
 *                 +---+   +---+   +---+ <----------------------------------
 *                   ^       ^       ^
 *                   |       |       |
 *                 +-----+-------+-----+--
 *   ------+-----> |     |       |     |  ....    <------ this is one pool
 *         |       +-----+-------+-----+--
 *         |
 *         |
 *         |       +---+   +---+ 
 *         |       |   |   |   | 
 *         |       |   |   |   |                   then there are three pools, each pool can
 *         |       +---+   +---+  .....            grow independently and dynamically.
 *         |       |   |   |   |
 *         |       |   |   |   |                   each pool has a lock for multi-thread, if
 *         |       +---+   +---+                   one pool is locked we can try next one
 *         |         ^       ^   
 *         |         |       |                     actually if use this class in single thread
 *         |       +-----+-----+--                 environment, then one pool is enough
 *         +-----> |     |     |  ....
 *         |       +-----+-----+--
 *         |
 *         |
 *         |       +---+   +---+   +---+
 *         |       |   |   |   |   |   |
 *         |       |   |   |   |   |   |
 *         |       +---+   +---+   +---+  .....
 *         |       |   |   |   |   |   |
 *         |       |   |   |   |   |   |
 *         |       +---+   +---+   +---+
 *         |         ^       ^       ^
 *         |         |       |       |
 *         |       +-----+-------+-----+--
 *         +-----> |     |       |     |  ....
 *                 +-----+-------+-----+--
 *        
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

#include <mutex>
#include <array>
#include <vector>
#include <cstdint>

#include "cachequeue.hpp"

// BlockSize : size of buffer allocated in byte
// BlockCount: how many buffer it can allocate for one static memory unit
// SlotCount : branches for multi-thread performance
// ThreadSafe: 
//
// And then there is a std::vector which contains many memory units, it dynamically
// increases to handle unlimited buffer allocation request, but most likely we only
// have a few memory uints in the vector
//
template<size_t UnitSize, size_t UnitCount, size_t BranchCount>
class MemoryBlockPN
{
    private:
        // tihs struct would never be allocated
        // just use for offset calculation
        typedef struct _MemoryChunk{
            bool In;            // this chunk is allocated in the memory pool

            size_t Offset;      // offset in its memory buffer
            size_t PoolID;      //
            size_t BranchID;    //

            uint8_t Data[1];    // offset to the data field
        }MemoryChunk;

        typedef struct _InnFBTree{
            uint8_t *Data;
            std::array<size_t, ((UnitCount << 1) - 1)> Longest;

            static size_t LeftLeaf(size_t nIndex)
            {
                return nIndex * 2 + 1;
            }

            static size_t RightLeaf(size_t nIndex)
            {
                return nIndex * 2 + 2;
            }

            static size_t ParentLeaf(size_t nIndex)
            {
                return (nIndex + 1) / 2 - 1;
            }

            static bool PowerOf2(size_t nParam)
            {
                return !(nParam & (nParam - 1));
            }

            // TODO & TBD
            // this is portable but not optimized, gcc has builtin functions
            // as __builtin_clz(), google it, however it's not portable
            static size_t FixSizeByPower2(size_t nSize)
            {
                nSize |= (nSize >> 1);
                nSize |= (nSize >> 2);
                nSize |= (nSize >> 4);

                if(sizeof(size_t) >= 2){ nSize |= (nSize >>  8); }
                if(sizeof(size_t) >= 4){ nSize |= (nSize >> 16); }
                if(sizeof(size_t) >= 8){ nSize |= (nSize >> 32); }

                return nSize + 1;
            }

            _InnFBTree()
            {
                size_t nNodeSize = UnitCount * 2;
                for(int nIndex = 0; nIndex < 2 * UnitCount - 1; ++nIndex){
                    if(PowerOf2(nIndex + 1)){
                        nNodeSize /= 2;
                    }

                    Longest[nIndex] = nNodeSize;
                }

                Data = new uint8_t[UnitSize * UnitCount];
            }

            ~_InnFBTree()
            {
                delete Data;
            }

            void *Get(size_t nSizeInByte)
            {
                constexpr auto pOff = (uint8_t *)(&((*((MemoryChunk *)(0))).Data[0]));
                size_t nSizeInUnit = ((size_t)(uintptr_t)pOff + nSizeInByte + UnitSize - 1) / UnitSize;

                // then nSizeInUnit == 0 won't happen
                if(!PowerOf2(nSizeInUnit)){
                    nSizeInUnit = FixSizeByPower2(nSizeInUnit);
                }

                // oooops, request tooo large memory chunk
                // dynamically allocate it and return directly
                if(nSizeInUnit > UnitCount){
                    auto pNewChunk = (MemoryChunk *)(new uint8_t[nSizeInUnit * UnitSize]);
                    pNewChunk->In = false;

                    return &(pNewChunk->Data[0]);
                }

                // ok try to allocate it in current pool
                for(size_t nNodeSizeInUnit = UnitCount, nIndex = 0;
                        nNodeSizeInUnit != nSizeInUnit; nNodeSizeInUnit /= 2){
                    size_t nLeftLeaf = LeftLeaf(nIndex);
                    size_t nRightLeaf = RightLeaf(nIndex);

                    if(Longest[nLeftLeaf] >= nNodeSizeInUnit){
                        if(Longest[nRightLeaf] >= nNodeSizeInUnit){
                            if(Longest[nLeftLeaf] <= Longest[nRightLeaf]){
                                nIndex = nLeftLeaf;
                            }else{
                                nIndex = nRightLeaf;
                            }
                        }else{
                            // TODO ????
                            nIndex = nRightLeaf;
                        }
                    }else{
                        nIndex = nRightLeaf;
                    }
                }
            }
        };

        using InnFBTree = _InnFBTree<UnitCount>




    private:
        // for RTTI to cooperate with thread-safe parameter
        // this class won't maintain the validation of the mutex it refers to
        typedef struct _InnLockGuard{
            std::mutex *Lock;

            _InnLockGuard(std::mutex *pLock)
                : Lock(pLock)
            {
                if(ThreadSafe){
                    Lock->lock();
                }
            }

            ~_InnLockGuard()
            {
                if(ThreadSafe){
                    Lock->unlock();
                }
            }
        }InnLockGuard;

        // structure of one buffer, which will be allocated by Get()
        // it will do for memory align automatically
        //
        // I put a field ``Extend" here to support multi-slot
        typedef struct _InnMemoryBlock{
            size_t Param;            // inside one memory unit
            size_t Extend;           // extend for memory pool array for multi-thread performance
            uint8_t Data[BlockSize]; // where we put the data
        }InnMemoryBlock;

        // structure of one memory unit
        typedef struct _InnMemoryUnit{
            CacheQueue<size_t, BlockCount> ValidQ;
            std::array<InnMemoryBlock, BlockCount> BlockV;

            _InnMemoryUnit(size_t nParam, size_t nExtend = 0)
            {
                // TODO & TBD
                // we can skip these to clears
                ValidQ.Clear();

                for(size_t nIndex = 0; nIndex < BlockCount; ++nIndex){
                    ValidQ.PushHead(nIndex);
                    BlockV[nIndex].Param = nParam;

                    // uncessary when slot is less than 2
                    // just put it here since it won't cause much during initialization
                    BlockV[nIndex].Extend = nExtend;
                }
            }

            void *InnGet()
            {
                if(ValidQ.Empty()){ return nullptr; }

                auto *pData = &(BlockV[ValidQ.Head()].Data[0]);
                ValidQ.PopHead();

                return pData;
            }
        }InnMemoryUnit;

        typedef struct _InnMemoryBlockPool{
            std::mutex *Lock;
            const size_t m_ID;
            std::vector<std::shared_ptr<InnMemoryUnit>> UnitV;

            _InnMemoryBlockPool(size_t nPoolID = 0)
                : Lock(ThreadSafe ? (new std::mutex()) : nullptr)
                , m_ID(nPoolID)
            {}

            ~_InnMemoryBlockPool()
            {
                delete Lock;
            }

            // this function never return nullptr
            void *InnGet()
            {
                for(auto pMU = UnitV.rbegin(); pMU != UnitV.rend(); ++pMU){
                    if(auto pRet = pMU->Get()){
                        return pRet;
                    }
                }

                // ooops, need to add a new unit
                UnitV.emplace_back(std::make_shared<InnMemoryUnit>(UnitV.size(), m_ID));
                return UnitV.back()->InnGet();
            }

            void *Get()
            {
                if(ThreadSafe){
                    if(Lock->try_lock()){
                        void *pRet = nullptr;

                        try{
                            pRet = InnGet();
                        }catch(...){
                            Lock->unlock();
                            return nullptr;
                        }

                        Lock->unlock();
                        return pRet;
                    }else{
                        // if we can't get hold current lock
                        // just return nullptr
                        return nullptr;
                    }
                }
                return InnGet();
            }
        }InnMemoryBlockPool;

    private:
        size_t m_Count;
        std::array<InnMemoryBlockPool, SlotCount> m_MPV;

    public:
        MemoryBlockPN()
            : m_Count(0)
        {
            static_assert(true
                    && BlockSize  > 0
                    && BlockCount > 0
                    && SlotCount  > 0, "invalid argument for MemoryBlockPN");
            }

        ~MemoryBlockPN() = default;

    public:
        void *Get()
        {
            // don't need those logics
            if(SlotCount == 1){ return m_MPV[0].Get(); }

            int nIndex = (m_Count++) % m_MPV.size();
            while(true){
                if(auto pRet = m_MPV[nIndex].Get()){
                    return pRet;
                }

                nIndex = (nIndex + 1) % m_MPV.size();
            }

            return nullptr;
        }

        void Free(void *pData)
        {
            if(!pData){ return; }

            // pOff is the constant offset of a memory block to its data field
            // pHead is the starting address of the memory block, w.r.t. the data chunk to free
            constexpr auto pOff  = (uint8_t *)(&((*((InnMemoryBlock *)(0))).Data[0]));
            const     auto pHead = (InnMemoryBlock *)((uint8_t *)pData - pOff);

            // 1. get the pool index
            size_t nPoolIndex = ((SlotCount > 1) ? (pHead->Extend) : 0);

            // 2. lock this pool if necessary
            InnLockGuard stInnLG(&(m_MPV[nPoolIndex].Lock));

            // 3. get the index of memory block in the memory unit
            //    the internal vector UnitV[...] may expand so don't put it as step-2
            size_t nUnitIndex = (pHead - 
                    &(m_MPV[nPoolIndex].UnitV[pHead->Param]->BlockV[0])) / sizeof(InnMemoryBlock);

            // 4. put it as valid
            m_MPV[nPoolIndex].ValidQ.PushHead(nUnitIndex);
        }
};
