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
#include "typecast.hpp"
#include "fflerror.hpp"

struct LocHashHelper
{
    size_t operator() (const std::tuple<int, int> &parm) const
    {
        const auto [x, y] = parm;
        if(x >= 0 && x < INT16_MAX && y >= 0 && y < INT16_MAX){
            return (size_t)(y) * (size_t)(INT16_MAX) + (size_t)(x);
        }
        throw fflerror("invalid location: x = %lld, y = %lld", to_lld(x), to_lld(y));
    }
};

template<typename T> using LocHashTable = std::unordered_map<std::tuple<int, int>, T, LocHashHelper>;
