/*
 * =====================================================================================
 *
 *       Filename: memoryblockpn.hpp
 *        Created: 05/12/2016 23:01:23
 *    Description: fixed size memory block pool
 *                 simple implementation for performance
 *                 thread safe is optional, but self-contained
 *
 *
 *                 +---+   +---+   +---+ <----------------------------------
 *                 |   |   |   |   |   | <------ this is one block       ^
 *                 |   |   |   |   |   |                                 |
 *                 +---+   +---+   +---+  .....                        this is a memory pool
 *                 |   |   |   |   |   |                                 |
 *                 |   |   |   |   |   |                                 V
 *                 +---+   +---+   +---+ <----------------------------------
 *                   ^       ^       ^
 *                   |       |       |
 *                 +-----+-------+-----+--
 *   ------+-----> |     |       |     |  ....    <------ this is one memory branch
 *         |       +-----+-------+-----+--
 *         |
 *         |
 *         |       +---+   +---+ 
 *         |       |   |   |   | 
 *         |       |   |   |   |                   then there are three branches, each branch can
 *         |       +---+   +---+  .....            grow independently and dynamically.
 *         |       |   |   |   |
 *         |       |   |   |   |                   each branch has a lock for multi-thread, if
 *         |       +---+   +---+                   one branch is locked we can try next one
 *         |         ^       ^   
 *         |         |       |                     actually if use this class in single thread
 *         |       +-----+-----+--                 environment, then one branch is enough
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
#include <vector>
#include <memory>
#include <cstdint>

#include "cachequeue.hpp"

// BlockSize  : size of memory block in byte, fixed
// PoolSize   : how many blocks in one pool, allocated statically
// BranchSize : branches for multi-thread performance, can be
//                  0   : invalid
//                  1   : single-thread
//               >= 2   : multi-thread environment
//
template<size_t BlockSize, size_t PoolSize, size_t BranchSize>
class MemoryBlockPN
{
    private:
        // for RTTI to cooperate with thread-safe parameter
        // this class won't maintain the validation of the mutex it refers to
        typedef struct _InnLockGuard{
            std::mutex *Lock;

            _InnLockGuard(std::mutex *pLock)
            {
                if(BranchSize > 1){
                    Lock = pLock;
                    Lock->lock();
                }
            }

            ~_InnLockGuard()
            {
                if(BranchSize > 1){
                    Lock->unlock();
                }
            }
        }InnLockGuard;

    private:
        // structure of one buffer, which will be allocated by Get()
        // it will do for memory align automatically
        //
        // I put a field ``BranchID" here to support multi-branches
        typedef struct _InnMemoryBlock{
            size_t PoolID;            // inside one branch, which pool it's in
            size_t BranchID;          // extend for memory branch
            uint8_t Data[BlockSize];  // where we put the data
        }InnMemoryBlock;

        // structure of one memory pool
        typedef struct _InnMemoryBlockPool{
            InnMemoryBlock BlockV[PoolSize];        // data position
            CacheQueue<size_t, PoolSize> ValidQ;    // marker queue

            // actually maybe we won't need to assign branch id here
            // but it's ok since this is only being done at initialization
            //
            // not like MemoryChunkPN which done at each allocation
            _InnMemoryBlockPool(size_t nPoolID, size_t nBranchID = 0)
            {
                // TODO & TBD
                // we can skip these to clears
                ValidQ.Clear();

                for(size_t nIndex = 0; nIndex < PoolSize; ++nIndex){
                    ValidQ.PushHead(nIndex);
                    BlockV[nIndex].PoolID = nPoolID;

                    if(BranchSize > 1){
                        BlockV[nIndex].BranchID = nBranchID;
                    }
                }
            }

            void *Get()
            {
                if(ValidQ.Empty()){ return nullptr; }

                auto *pData = &(BlockV[ValidQ.Head()].Data[0]);
                ValidQ.PopHead();

                return pData;
            }

            // TODO & TBD
            // we don't have a Free() here
        }InnMemoryBlockPool;

    private:
        typedef struct _InnMemoryBlockPoolBranch{
            size_t BranchID;
            std::mutex *Lock;
            std::vector<std::shared_ptr<InnMemoryBlockPool>> PoolV;

            // didn't assign BranchID here, it's in a raw array
            // and hard to use ctor with parameters, has to initialize it manually
            _InnMemoryBlockPoolBranch()
            {
                // we are saving anything we can in this class
                // even didn't assign Lock as zero if BranchSize == 0
                if(BranchSize > 1){
                    Lock = new std::mutex();
                }
            }

            ~_InnMemoryBlockPoolBranch()
            {
                if(BranchSize > 1){
                    delete Lock;
                }
            }

            // this function never return nullptr
            void *InnGet()
            {
                for(auto pMP = PoolV.rbegin(); pMP != PoolV.rend(); ++pMP){
                    if(auto pRet = (*pMP)->Get()){
                        return pRet;
                    }
                }

                // ooops, need to add a new pool
                PoolV.emplace_back(std::make_shared<InnMemoryBlockPool>(PoolV.size(), (BranchSize > 1) ? BranchID : 0));
                return PoolV.back()->Get();
            }

            void *Get()
            {
                if(BranchSize > 1){
                    void *pRet = nullptr;

                    if(Lock->try_lock()){
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

                // it's single thread so
                return InnGet();
            }
        }InnMemoryBlockPoolBranch;

    private:
        size_t m_Count;
        InnMemoryBlockPoolBranch m_MBPBV[BranchSize];

    public:
        MemoryBlockPN()
            : m_Count(0)
        {
            static_assert(true
                    && BlockSize  > 0
                    && PoolSize   > 0
                    && BranchSize > 0, "invalid argument for MemoryBlockPN");

            // need to assign BranchID here
            // number of branch is fixed so it's can be done in ctor
            for(size_t nBranchID = 0; nBranchID < BranchSize; ++nBranchID){
                m_MBPBV[nBranchID].BranchID = nBranchID;
            }
        }

        ~MemoryBlockPN() = default;

    public:
        void *Get()
        {
            // don't need those logics
            if(BranchSize == 1){ return m_MBPBV[0].Get(); }

            // OK it's in multi-thread environment
            // I didn't protect m_Count since it's just a huristic variable
            int nIndex = (m_Count++) % BranchSize;
            while(true){
                if(auto pRet = m_MBPBV[nIndex].Get()){
                    return pRet;
                }

                nIndex = (nIndex + 1) % BranchSize;
            }

            // to make the compiler happy
            return nullptr;
        }

        void Free(void *pData)
        {
            if(!pData){ return; }

            // pOff is the constant offset of a memory block to its data field
            // pHead is the starting address of the memory block, w.r.t. the data chunk to free
            constexpr auto pOff  = (uint8_t *)(&((*((InnMemoryBlock *)(0))).Data[0]));
            const     auto pHead = (InnMemoryBlock *)((uint8_t *)pData - pOff);

            // 1. get the branch index
            size_t nBranchIndex = ((BranchSize > 1) ? (pHead->BranchID) : 0);

            // 2. lock this pool if necessary
            //    if BranchSize == 1 this lock will do nothing
            InnLockGuard stInnLG(m_MBPBV[nBranchIndex].Lock);

            // 3. get the index of memory block in the memory pool
            //    the internal vector PoolV[...] may expand so don't put it as step-2
            //
            //    since it's already pointer of type InnMemoryBlock
            //    no need for devision of sizeof(InnMemoryBlock)
            size_t nBlockID = pHead - &(m_MBPBV[nBranchIndex].PoolV[pHead->PoolID]->BlockV[0]);

            // 4. put it as valid
            m_MBPBV[nBranchIndex].PoolV[pHead->PoolID]->ValidQ.PushHead(nBlockID);
        }
};
