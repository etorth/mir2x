#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <type_traits>

namespace hexstr
{
    // convert an intergal key to a hex string
    // 1. invocation should prepare enough buffer to strBuf
    // 2. no '\0' padding at the end for compactness
    template<typename T, size_t ByteN = 0> const char *to_string(T key, char *strBuf, bool appendZero)
    {
        static_assert(std::is_unsigned<T>::value, "hexstr::to_string() requires unsigned intergal type");
        constexpr uint16_t hexStrChunk[]
        {
            #include "hexstr.inc"
        };

        constexpr size_t fillByteN = (ByteN) ? (std::min<size_t>)(ByteN, sizeof(T)) : sizeof(T);
        for(size_t i = 0; i < fillByteN; ++i, (key >>= 8)){
            std::memcpy(strBuf + 2 * (fillByteN - i - 1), &(hexStrChunk[(key & 0XFF)]), sizeof(uint16_t));
        }

        if(appendZero){
            strBuf[fillByteN * 2] = '\0';
        }
        return strBuf;
    }

    // convert a const string into an intergal key
    // 1. ByteN indicate how many bytes to convert, i.e.
    //    to_hex<uint32_t, 1>("123456") will return 0X12
    //    to_hex<uint32_t, 2>("123456") will return 0X1234
    //    to_hex<uint32_t, 3>("123456") will return 0X123456
    //    to_hex<uint32_t, 4>("123456") will return 0X12345600
    template<typename T, size_t ByteN = 0> T to_hex(const char *strBuf)
    {
        static_assert(std::is_unsigned<T>::value, "hexstr::to_hex() requires unsigned intergal type");
        constexpr uint8_t strHexChunk[]
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

        constexpr size_t fillByteN = (ByteN) ? (std::min<size_t>)(ByteN, sizeof(T)) : sizeof(T);
        T result = 0;

        for(size_t i = 0; strBuf[i] != '\0' && i < fillByteN * 2; ++i){
            result = (result << 4) + strHexChunk[strBuf[i] - '0'];
        }
        return result;
    }
}
