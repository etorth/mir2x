/*
 * =====================================================================================
 *
 *       Filename: lochashtable.hpp
 *        Created: 05/20/2020 10:44:04
 *    Description: 
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
#include <utility>
#include <cstdint>
#include <unordered_map>
#include "totype.hpp"
#include "fflerror.hpp"

struct LocHashHelper
{
    size_t operator() (const std::tuple<int, int> &parm) const
    {
        const auto [x, y] = parm;
        if(x < 0 || y < 0){
            throw fflerror("invalid location: x = %d, y = %d", x, y);
        }
        return to_uz((to_u64(y) << 32) | to_u64(x));
    }
};

template<typename T> using LocHashTable = std::unordered_map<std::tuple<int, int>, T, LocHashHelper>;
