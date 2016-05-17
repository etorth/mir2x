/*
 * =====================================================================================
 *
 *       Filename: memoryblockpool.hpp
 *        Created: 05/12/2016 23:01:23
 *  Last Modified: 05/16/2016 19:58:53
 *
 *    Description: fixed size memory block pool
 *                 simple implementation for performance
 *                 thread safe is optional, but self-contained
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

// BlockSize : size of uint8_t for the allocated buffer
// BlockCount: how many buffer it can allocate for one memory unit
// ThreadSafe: 
//
// And then there is a std::vector which contains many memory units, it dynamically
// increases to handle unlimited buffer allocation request, but most likely we only
// have a few memory uints in the vector
//
template<size_t BlockSize, size_t BlockCount, bool ThreadSafe>
class MemoryBlockPool
{
    private:
        // structure of one buffer, which will be allocated by Get()
        // it will do for memory align automatically
        typedef _InnMemoryBlock{
            size_t Param;
            uint8_t Data[BlockSize];
        }InnMemoryBlock;

        // structure of one memory unit
        typedef _InnMemoryBlockV{
            std::mutex *Mutex;
            CacheQueue<size_t, BlockCount> UsedQ;
            CacheQueue<size_t, BlockCount> UnusedQ;
            std::array<InnMemoryBlock, BlockCount> BlockV;

            _InnMemoryBlockV(size_t nParam)
                : Mutex(nullptr)
            {
                // TODO & TBD
                // we can skip these to clears
                UsedQ.Clear();
                UnusedQ.Clear();

                for(size_t nIndex = 0; nIndex < BlockCount; ++nIndex){
                    UnusedQ.PushHead(nIndex);
                    BlockV[nIndex].Param = nParam;
                }

                if(ThreadSafe){
                    Mutex = new std::mutex();
                }
            }

            void *Get()
            {
                if(UnusedQ.Empty()){ return nullptr; }

                auto *pData = &(BlockV[UnusedQ.Head()].Data[0]);
                UnusedQ.PopHead();

                return (void *)pData;
            }
        }InnMemoryBlockV;

    private:
        std::vector<InnMemoryBlockV> m_MBVV;

    public:
        MemoryBlockPool()
            : m_MBVV(1, 0)
        {
            static_assert(BlockSize > 0, "non-empty buffer allocation please");
            static_assert(BlockCount > 0, "non-empty buffer pool unit supported only");
        }

        virtual ~MemoryBlockPool();

    public:
        void *Get()
        {
            // if the memory holds for a pretty long time, then maybe here we need
            // further optimization, rather than this O(n) approach
            //
            // for fast allocate-free usage, it's OK since most of the time we only
            // have one MBV in m_MBVV;
            //
            // maybe we can use reverse iterator with assumption that last MBV is
            // more likely to be valid
            for(auto &rstMBV: m_MBVV){
                auto pData = rstMBV.Get();
                if(pData){ return pData; }
            }

            m_MBVV.emplace_back(m_MBVV.size());
            return m_MBVV.back().Get();
        }

        void Free(void *pData)
        {
            const auto pOff  = (uint8_t *)(&((*((InnMemoryBlock *)(0))).Data[0]));
            const auto pHead = (InnMemoryBlock *)((uint8_t *)pData - pOff);

            // TODO & TBD
            // here m_MBVV.capacity() is non-decreasing
            // so we assume it's always OK
            m_MBVV[pHead->Param].UnusedQ.PushHead(
                    (pHead - &(m_MBVV[pHead->Param].BlockV[0])) / sizeof(InnMemoryBlock));
        }
};
