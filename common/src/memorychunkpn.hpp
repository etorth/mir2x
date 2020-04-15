/*
 * =====================================================================================
 *
 *       Filename: memorychunkpn.hpp
 *        Created: 05/12/2016 23:01:23
 *    Description: unfixed-size memory chunk pool, thread safe is optional, but self-contained
 *                 this algorithm is based on buddy algorithm
 *
 *                 TODO: get ride of strict aliasing issue
 *
 *                 copy from https://github.com/wuwenbin/buddy2
 *
 *
 *                 +---+   +---+   +---+ <----------------------------------
 *                 |   |   |   |   |   | <------ this is one chunk       ^
 *                 +---+   |   |   +---+                                 |
 *                 |   |   +---+   |   |  .....                        this is one memory pool
 *                 +---+   +---+   |   |                                 |
 *                 |   |   |   |   |   |                                 V
 *                 +---+   +---+   +---+ <----------------------------------
 *                   ^       ^       ^
 *                   |       |       |
 *                 +-----+-------+-----+--
 *   ------+-----> |     |       |     |  ....    <------ this is one branch
 *         |       +-----+-------+-----+--
 *         |
 *         |
 *         |       +---+   +---+
 *         |       |   |   |   |
 *         |       +---+   |   |                   then there are three branches, each branch can
 *         |       |   |   +---+  .....            grow independently and dynamically.
 *         |       +---+   +---+
 *         |       |   |   |   |                   each branch has a lock for multi-thread, if
 *         |       +---+   +---+                   one branch is locked we can try next one
 *         |         ^       ^
 *         |         |       |                     actually if use this class in single thread
 *         |       +-----+-----+--                 environment, then one branch is enough
 *         +-----> |     |     |  ....
 *         |       +-----+-----+--
 *         |                                       so when using in single thread environment, set
 *         |                                       BranchSize = 1
 *         |       +---+   +---+   +---+
 *         |       |   |   |   |   |   |
 *         |       +---+   |   |   +---+
 *         |       |   |   +---+   |   |  .....
 *         |       +---+   +---+   |   |
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
#include <memory>
#include <cstdint>
#include <cstddef>

#include "mathf.hpp"
#include "cachequeue.hpp"

// UnitSize   :    size of unit in bytes, this is the basic units for buffer length
// PoolSize   :    how many units in one pool
// BranchSize :    how many branchs in this PN, for different size
//                      0 :  invalid
//                      1 :  single thread
//                    >=2 :  multi-thread
//                 since when using multi-thread we can use multiple branches to avoid
//                 to wait the lock, otherwise all request need to wait for one lock,
//                 this helps to performance, however, for BranchSize == 1 there is no
//                 need for thread safity
//
template<size_t UnitSize, size_t PoolSize, size_t BranchSize>
class MemoryChunkPN
{
    private:
        // inner RTTI support, make it as simple as possible, to
        // make compiler has a chance to optimize it out when possible
        typedef struct _InnLockGuard{
            std::mutex *Lock;

            _InnLockGuard(std::mutex *pLock)
            {
                if(BranchSize > 1){
                    Lock = pLock;
                    if(Lock){
                        Lock->lock();
                    }
                }
            }

            ~_InnLockGuard()
            {
                if(BranchSize > 1){
                    if(Lock){
                        Lock->unlock();
                    }
                }
            }
        }InnLockGuard;

        // this struct would never be allocated, just use for offset calculation, maybe
        // I need to use uint8_t for saving space, but it's OK since we are avoiding
        // frequent allocation-dellocation, the memory usage efficiency is next.
        typedef struct _InnMemoryChunk{
            bool In;            // this to indicate the chunk is allocated in the memory pool
                                // actually we can compare pointer range to decide

            size_t NodeID;      // node in its memory pool, use it for offset calculation
            size_t PoolID;      //
            size_t BranchID;    //

            uint8_t Data[4];    // offset to the data field, size is always ok
        }InnMemoryChunk;

        typedef struct _InnMemoryChunkPool{
            // big piece of memory, can allocate independently, since anyway
            // this pool will be created dynamically, I just put it here to
            // make the CPU cache happy
            uint8_t Data[UnitSize * PoolSize];

            // this *Longest* array is the most important data structure, each
            // node has an *longest* valid memory to take in charge, here valid
            // means its not allocated previously, and it's on two buddies
            size_t Longest[PoolSize * 2 - 1];

            size_t PoolID;
            size_t BranchID;

            static size_t LeftNode(size_t nIndex)
            {
                return nIndex * 2 + 1;
            }

            static size_t RightNode(size_t nIndex)
            {
                return nIndex * 2 + 2;
            }

            static size_t ParentNode(size_t nIndex)
            {
                return (nIndex + 1) / 2 - 1;
            }

            // assume nIndex is in [0, 256 * 2 - 2)
            static size_t Level(size_t nIndex)
            {
                const static size_t kLevelV[] = {
                    0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4,
                    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5,
                    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6,
                    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9};
                return kLevelV[nIndex];
            }

            // TODO & TBD assume nNodeID is in [0, 256 * 2 - 2)
            static size_t ValidUnit(size_t nNodeID)
            {
                return (1 << (Level(2 * PoolSize - 2) - Level(nNodeID)));
            }

            // I didn't put branch ID here since not sure it's multi-branches for
            // multi-thread or only single branch for single thread
            _InnMemoryChunkPool(size_t nPoolID)
                : PoolID(nPoolID)
            {
                // or we can just use function ValidUnit(nIndex)
                size_t nNodeSize = PoolSize * 2;
                for(size_t nIndex = 0; nIndex < 2 * PoolSize - 1; ++nIndex){
                    if(mathf::powerOf2(nIndex + 1)){
                        nNodeSize /= 2;
                    }

                    Longest[nIndex] = nNodeSize;
                }
            }

            // all static, nothing to destruct
            ~_InnMemoryChunkPool() = default;

            // TODO & TBD
            // internal use, so won't check the validation of the parameter
            //
            // assume: 1. nSizeInUnit is proper
            //         2. nSizeInUnit is in power of 2
            void *Get(size_t nSizeInUnit)
            {
                // this pool can satisfy the passing allcation request i.i.f Longest[0]
                // is >= nSizeInUnit
                //
                // failed
                if(Longest[0] < nSizeInUnit){ return nullptr; }

                // OK now there is a continuous memory region, we need to find it
                // and use (part of) it to this allocation request
                size_t nIndex = 0;
                size_t nLevelNodeSize = PoolSize;

                // the loop will always reach the level where nLevelNodeSize == nSizeInUnit, means
                // if a chunk will be allocated, it will always allocated at the corresponding
                // level, not higher, not lower, and if it can't, it will be refused at level-0
                //
                // do some reasoning here, don't just accept it's true
                //
                while(nLevelNodeSize != nSizeInUnit){
                    size_t nLeftNode = LeftNode(nIndex);
                    size_t nRightNode = RightNode(nIndex);

                    // keep in mind that it's impossible that both child nodes are
                    // smaller than nSizeInUnit in the loop
                    if(Longest[nLeftNode] < Longest[nRightNode]){
                        if(Longest[nLeftNode] >= nSizeInUnit){
                            nIndex = nLeftNode;
                        }else{
                            nIndex = nRightNode;
                        }
                    }else{
                        if(Longest[nRightNode] >= nSizeInUnit){
                            nIndex = nRightNode;
                        }else{
                            nIndex = nLeftNode;
                        }
                    }

                    nLevelNodeSize /= 2;
                }

                // ok now we are in the proper level and proper node
                Longest[nIndex] = 0;
                size_t nOffset = (nIndex + 1) * nLevelNodeSize - PoolSize;

                // mark it in the structure pointer, in the wuwenbin/buddy2 implementation, the auther
                // return the offset and find the proper nNodeID whose Longest[nNodeID] == 0, but here
                // I record the nNodeID directly to avoid the retrieving
                //
                auto pHead = ((InnMemoryChunk *)(Data + nOffset * UnitSize));
                pHead->In = true;
                pHead->NodeID = nIndex;
                pHead->PoolID = PoolID;

                if(BranchSize > 1){
                    pHead->BranchID = BranchID;
                }

                // set all its ancestor for right capacity
                while(nIndex){
                    nIndex = ParentNode(nIndex);
                    Longest[nIndex] = (std::max<size_t>)(Longest[LeftNode(nIndex)], Longest[RightNode(nIndex)]);
                }

                return &(pHead->Data[0]);
            }

            // internal use, so there is no parameter check
            void Free(size_t nNodeID)
            {
                // TODO: by our implementation, for one nNodeID to free, its Longest should
                //       always be zero to indicate it's already allocated before
                //
                //       so actually here we should throw an exception since it's to free a
                //       node which never be allocated!
                //
                if(Longest[nNodeID]){ return; }

                // 1. recover the full state of this node
                //    when pool size is N, then node count is (2 * N - 1)
                //    and maximal node index is (2 * N - 2)

                Longest[nNodeID] = ValidUnit(nNodeID);

                while(nNodeID){
                    // 1. move to parent node
                    nNodeID = ParentNode(nNodeID);

                    // 2. get protential valid unit count for this node
                    size_t nUnit = ValidUnit(nNodeID);

                    // 3. its two children
                    size_t nLeftUnit = Longest[LeftNode(nNodeID)];
                    size_t nRightUnit = Longest[RightNode(nNodeID)];


                    if(nLeftUnit + nRightUnit == nUnit){
                        // ok now left and right are both *fully* valid
                        // means no memory in left and right node is allocated
                        Longest[nNodeID] = nUnit;
                    }else{
                        // part or all of the memory it takes in charge is not valid
                        // mark for the longest valid memory
                        Longest[nNodeID] = (std::max<size_t>)(nLeftUnit, nRightUnit);
                    }
                }
            }
        }InnMemoryChunkPool;

    private:
        typedef struct _InnMemoryChunkPoolBranch{
            size_t BranchID;
            std::mutex *Lock;
            std::vector<std::shared_ptr<InnMemoryChunkPool>> PoolV;

            _InnMemoryChunkPoolBranch()
            {
                // even I won't zero it when in single thread
                if(BranchSize > 1){
                    Lock = new std::mutex();
                }
            }

            ~_InnMemoryChunkPoolBranch()
            {
                if(BranchSize > 1){
                    delete Lock;
                }
            }

            // this function never fail, it always get an memory piece
            // given proper nSizeInUnit
            void *InnGet(size_t nSizeInUnit)
            {
                for(auto pMP = PoolV.rbegin(); pMP != PoolV.rend(); ++pMP){
                    if(auto pRet = (*pMP)->Get(nSizeInUnit)){
                        return pRet;
                    }
                }

                // ooops, need to add a new unit
                PoolV.emplace_back(std::make_shared<InnMemoryChunkPool>(PoolV.size()));
                if(BranchSize > 1){
                    PoolV.back()->BranchID = BranchID;
                }

                return PoolV.back()->Get(nSizeInUnit);
            }

            void *Get(size_t nSizeInUnit)
            {
                if(BranchSize > 1){
                    void *pRet = nullptr;

                    if(Lock->try_lock()){
                        try{
                            pRet = InnGet(nSizeInUnit);
                        }catch(...){
                            Lock->unlock();
                            return nullptr;
                        }

                        Lock->unlock();
                        return pRet;
                    }else{
                        // if current thread can't get current lock
                        // just return nullptr
                        return nullptr;
                    }
                }

                // ok just in single thread...
                return InnGet(nSizeInUnit);
            }

            // TODO & TBD
            // no Free() function defined
        }InnMemoryChunkPoolBranch;

    private:
        size_t m_Count;
        InnMemoryChunkPoolBranch m_MCPBV[BranchSize];

    public:
        MemoryChunkPN()
            : m_Count(0)
        {
            // 1. check parameters
            static_assert(UnitSize > 0 && UnitSize <= 256 && (!(UnitSize & (UnitSize - 1))), "invalid argument for UnitSize");
            static_assert(PoolSize > 0 && PoolSize <= 256 && (!(PoolSize & (PoolSize - 1))), "invalid argument for PoolSize");
            static_assert(BranchSize > 0, "invalid argument for BranchSize");

            // 2. assign branch id to each branch if necessary
            if(BranchSize > 1){
                for(size_t nIndex = 0; nIndex < BranchSize; ++nIndex){
                    m_MCPBV[nIndex].BranchID = nIndex;
                }
            }
        }

        ~MemoryChunkPN() = default;

    public:
        void *Get(size_t nSizeInByte)
        {
            constexpr auto nOff = offsetof(InnMemoryChunk, Data);
            size_t nSizeInUnit = (nSizeInByte + nOff + UnitSize - 1) / UnitSize;

            // then nSizeInUnit == 0 won't happen
            if(!mathf::powerOf2<size_t>(nSizeInUnit)){
                nSizeInUnit = mathf::roundByPowerOf2<size_t>(nSizeInUnit);
            }

            // oooops, request tooo large memory chunk and any pool can't satisfy
            // dynamically allocate it and return
            if(nSizeInUnit > PoolSize){
                auto pNewChunk = (InnMemoryChunk *)(new uint8_t[nSizeInUnit * UnitSize]);
                pNewChunk->In = false;

                return &(pNewChunk->Data[0]);
            }

            // size is ok, then now we'll try to allocated in memory pools

            if(BranchSize == 1){ return m_MCPBV[0].Get(nSizeInUnit); }

            size_t nIndex = (m_Count++) % BranchSize;
            while(true){
                if(auto pRet = m_MCPBV[nIndex].Get(nSizeInUnit)){
                    return pRet;
                }

                nIndex = (nIndex + 1) % BranchSize;
            }

            // to make the compiler happy
            return nullptr;
        }

        template<typename T> T *Get()
        {
            return (T *)(Get(sizeof(T)));
        }

        void Free(void *pBuf)
        {
            if(!pBuf){ return; }

            auto pHead = (InnMemoryChunk *)((uint8_t *)pBuf - offsetof(InnMemoryChunk, Data));
            if(!pHead->In){
                delete [] (uint8_t *)pHead; return;
            }

            // when branch count is 1, the branch id is not set
            if(BranchSize == 1){
                m_MCPBV[0].PoolV[pHead->PoolID]->Free(pHead->NodeID);
                return;
            }

            // ok this is in multi-thread environment, we need the lock
            InnLockGuard(m_MCPBV[pHead->BranchID].Lock);
            m_MCPBV[pHead->BranchID].PoolV[pHead->PoolID]->Free(pHead->NodeID);
        }
};
