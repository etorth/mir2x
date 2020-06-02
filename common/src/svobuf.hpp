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
#include "short_alloc.h"

template<typename T, size_t N> class svobuf
{
    private:
        typename std::vector<T, short_alloc<T, N * sizeof(T), alignof(T)>>::allocator_type::arena_type m_alloc;

    public:
        std::vector<T, short_alloc<T, N * sizeof(T), alignof(T)>> c;

    public:
        svobuf(): c(m_alloc) {}
};
