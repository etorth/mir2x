/*
 * =====================================================================================
 *
 *       Filename: constexprf.hpp
 *        Created: 08/05/2017 12:14:11
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
#include <string_view>
namespace constexprf
{
    template<typename... U> constexpr bool hasInt(int var, int candidate, U&&... u)
    {
        return (var == candidate) || hasInt(var, std::forward<U>(u)...);
    }

    template<> constexpr bool hasInt(int var, int candidate)
    {
        return var == candidate;
    }

    // map2Int(szStr, D_NONE,
    //      u8"攻击", D_ATTACK,
    //      u8"挨打", D_HITTEF,
    //      u8"行走", D_WALK)
    template<typename... U> constexpr int map2Int(const char8_t *s, int defVar, const char8_t *optStr, int optVar, U&&... u)
    {
        return (std::u8string_view(s) == optStr) ? optVar : map2Int(s, defVar, std::forward<U>(u)...);
    }

    template<> constexpr int map2Int(const char8_t *s, int defVar, const char8_t *optStr, int optVar)
    {
        return (std::u8string_view(s) == optStr) ? optVar : defVar;
    }
}
