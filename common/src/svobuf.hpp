/*
 * =====================================================================================
 *
 *       Filename: svobuf.hpp
 *        Created: 03/29/2019 04:54:20
 *    Description: 
 *                should use std::align_storage not std::array
 *                std::array calls T()
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
#include <vector>
#include "fflerror.hpp"
#include "short_alloc.h"

template<typename T, size_t N> class svobuf
{
    private:
        typename std::vector<T, short_alloc<T, N * sizeof(T), alignof(T)>>::allocator_type::arena_type m_alloc;

    public:
        std::vector<T, short_alloc<T, N * sizeof(T), alignof(T)>> c;

    public:
        svobuf()
            : c(m_alloc)
        {
            // immediately use all static buffer
            // 1. to prevent push_back() to allocate on heap
            // 2. to prevent waste of memory

            // when we call push_back() without reserve
            // it a) allocate bigger buffer, then b) copy data, then c) deallocate current buffer
            // the short_alloc can't reuse deallocated buffer since it only increses linearly
            // if we use reserve then there is no waste caused by this

            // for a more powerful allocator can reuse the deallocated buffer, check this one:
            //
            //     https://github.com/orlp/libop/blob/master/bits/memory.h
            //
            // it works same as short_alloc but has ``free-list`` support

            // side effect is:
            //
            //     svobuf<int, 4> buf;
            //     auto b = buf.c;
            //
            //     b.push_back(1);
            //     b.push_back(2);
            //
            // here variable buf has ran out of buf.m_alloc static buffer
            // then b.push_back() will always allocates on heap

            // don't do this:
            //
            //    auto f()
            //    {
            //        svobuf<int, 4> buf;
            //        buf.c.push_back(1);
            //
            //        return buf.c;
            //    }
            //
            // we return a copy of buf.c, which copies of the allocator inside buf.c
            // but copy constructor of buf.c.allocator is simple a ref-bind to buf.m_alloc, then we have dangling ref

            c.reserve(N);
            if(c.capacity() > N){
                throw fflerror("allocate initial buffer dynamically");
            }
        }

    public:
        constexpr size_t svocap() const
        {
            return N;
        }
};
