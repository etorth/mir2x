/*
 * =====================================================================================
 *
 *       Filename: hexstring.hpp
 *        Created: 02/06/2016 13:35:51
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
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace HexString
{
    // convert an intergal key to a hex string
    // 1. invocation should prepare enough buffer to szString
    // 2. no '\0' padding at the end for compactness
    template<typename T, size_t ByteN = 0> const char *ToString(T nKey, char *szString)
    {
        static_assert(std::is_unsigned<T>::value, "HexString::ToString() requires unsigned intergal type");
        const size_t nByteN = (ByteN) ? std::min(ByteN, sizeof(T)) : sizeof(T);
        const uint16_t knvHexStringChunk[]
        {
            #include "hexstring.inc"
        };

        for(size_t nIndex = 0; nIndex < nByteN; ++nIndex, (nKey >>= 8)){
            *(uint16_t *)(szString + 2 * (nByteN - nIndex - 1)) = knvHexStringChunk[(nKey & 0XFF)];
        }

        return szString;
    }

    // convert a const string into an intergal key
    // 1. ByteN indicate how many bytes to convert, i.e.
    //    ToHex<uint32_t, 1>("123456") will return 0X12
    //    ToHex<uint32_t, 2>("123456") will return 0X1234
    //    ToHex<uint32_t, 3>("123456") will return 0X123456
    //    ToHex<uint32_t, 4>("123456") will return 0X12345600
    template<typename T, size_t ByteN = 0> T ToHex(const char *szString)
    {
        static_assert(std::is_unsigned<T>::value, "HexString::ToHex() requires unsigned intergal type");
        const size_t nByteN = (ByteN) ? std::min(ByteN, sizeof(T)) : sizeof(T);
        const uint8_t knvStringHexChunk[]
        {
            0X00,  // "0" - "0"
            0X01,  // "1" - "0"
            0X02,  // "2" - "0"
            0X03,  // "3" - "0"
            0X04,  // "4" - "0"
            0X05,  // "5" - "0"
            0X06,  // "6" - "0"
            0X07,  // "7" - "0"
            0X08,  // "8" - "0"
            0X09,  // "9" - "0"
            0XFF,  // ":" - "0" invalid
            0XFF,  // ";" - "0" invalid
            0XFF,  // "<" - "0" invalid
            0XFF,  // "=" - "0" invalid
            0XFF,  // ">" - "0" invalid
            0XFF,  // "?" - "0" invalid
            0XFF,  // "@" - "0" invalid
            0X0A,  // "A" - "0"
            0X0B,  // "B" - "0"
            0X0C,  // "C" - "0"
            0X0D,  // "D" - "0"
            0X0E,  // "E" - "0"
            0X0F,  // "F" - "0"
        };

        T nRes = 0;
        for(size_t nIndex = 0; szString[nIndex] != '\0' && nIndex < nByteN * 2; ++nIndex){
            nRes = (nRes << 4) + knvStringHexChunk[szString[nIndex] - '0'];
        }
        return nRes;
    }
}
