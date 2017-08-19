/*
 * =====================================================================================
 *
 *       Filename: pushstream.hpp
 *        Created: 08/17/2017 19:32:15
 *  Last Modified: 08/18/2017 16:54:24
 *
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

namespace PushStream
{
    inline void PushBit(std::vector<uint8_t> &stvByte, const std::vector<bool> &stvMark)
    {
        size_t nIndex = 0;
        while(nIndex < stvMark.size()){
            uint8_t nRes = 0X00;
            for(int nBit = 0; nBit < 8; ++nBit){
                nRes = (nRes >> 1) + ((nIndex < stvMark.size() && stvMark[nIndex++]) ? 0X80 : 0X00);
            }
            stvByte.push_back(nRes);
        }
    }

    inline bool PickOneBit(const uint8_t *pData, size_t nOffset)
    {
        return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) ? true : false;
    }

    template<typename T> void PushByte(std::vector<uint8_t> &stvData, const T &rstT)
    {
        static_assert(std::is_pod<T>(), "Require pod types to streamlize");
        auto pByte = (const uint8_t *)(&rstT);
        stvData.insert(stvData.end(), pByte, pByte + sizeof(T));
    }

    template<typename Iterator> void PushByte(std::vector<uint8_t> &stvData, Iterator pBegin, Iterator pEnd)
    {
        using type_remove_ref = typename std::remove_reference<decltype(*pBegin)>::type;
        using type_remove_cv  = typename std::remove_cv<type_remove_ref>::type;

        static_assert(std::is_same<type_remove_cv, uint8_t>(), "Invalid iterator type");
        stvData.insert(stvData.end(), pBegin, pEnd);
    }
}
