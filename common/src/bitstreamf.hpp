/*
 * =====================================================================================
 *
 *       Filename: bitstreamf.hpp
 *        Created: 08/17/2017 19:32:15
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
#include <vector>
#include <cstdint>

namespace bitstreamf
{
    template<typename T> void pushByte(std::vector<uint8_t> &data, const T &val)
    {
        static_assert(std::is_standard_layout_v<T>);
        const auto p = (const uint8_t *)(&val);
        data.insert(data.end(), p, p + sizeof(val));
    }

    template<typename Iterator> void pushByte(std::vector<uint8_t> &data, Iterator ibegin, Iterator iend)
    {
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(*ibegin)>, uint8_t>);
        data.insert(data.end(), ibegin, iend);
    }
}
