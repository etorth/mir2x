/*
 * =====================================================================================
 *
 *       Filename: utf8f.cpp
 *        Created: 12/12/2018 07:27:12
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
#include <utf8.h>
#include <cstring>
#include <cstdint>
#include <cinttypes>
#include <stdexcept>
#include "strf.hpp"
#include "utf8f.hpp"
#include "fflerror.hpp"

uint32_t utf8f::peekUTF8Code(const char *szUTF8String)
{
    // seems utf8::peek_next() is not what I need here
    // what it returns?

    if(!szUTF8String){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    size_t nStrLen = std::strlen(szUTF8String);
    if(nStrLen == 0){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: empty string"));
    }

    auto pszBegin = szUTF8String;
    auto pszEnd   = pszBegin;

    try{
        utf8::advance(pszEnd, 1, szUTF8String + nStrLen);
    }catch(...){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: failed to peek the first utf8 code"));
    }

    if(pszEnd - pszBegin > 4){
        throw std::runtime_error(str_fflprintf(": Pick a code point longer than 4 bytes: %s", szUTF8String));
    }

    uint32_t nUTF8Key = 0;
    std::memcpy(&nUTF8Key, pszBegin, pszEnd - pszBegin);
    return nUTF8Key;
}

std::vector<int> utf8f::buildUTF8Off(const char *utf8String)
{
    if(!utf8String){
        throw fflerror("null utf8String");
    }

    const auto strLen = std::strlen(utf8String);
    if(strLen == 0){
        return {};
    }

    std::vector<int> off;

    const char *pszCurr = utf8String;
    const char *pszEnd  = utf8String + strLen;

    off.reserve(strLen);
    for(; pszCurr < pszEnd; utf8::advance(pszCurr, 1, pszEnd)){
        off.push_back(pszCurr - utf8String);
    }

    return off;
}
