/*
 * =====================================================================================
 *
 *       Filename: hexstring.cpp
 *        Created: 02/15/2016 17:18:36
 *  Last Modified: 03/11/2016 22:47:23
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
#include "hexstring.hpp"

uint32_t StringHex(const char *szStr, size_t nLen)
{
    const uint8_t knvStringHexChunk[] = {
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
    uint32_t nKey = 0;
    for(size_t nIndex = 0; szStr[nIndex] != '\0' && nIndex < nLen; ++nIndex){
        nKey = (nKey << 4) + knvStringHexChunk[szStr[nIndex] - '0'];
    }
    return nKey;
}
