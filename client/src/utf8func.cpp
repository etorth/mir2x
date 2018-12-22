/*
 * =====================================================================================
 *
 *       Filename: utf8func.cpp
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
#include "strfunc.hpp"
#include "utf8func.hpp"

uint32_t UTF8Func::PeekUTF8Code(const char *szUTF8String)
{
    if(!szUTF8String){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    try{
        return utf8::peek_next(szUTF8String, szUTF8String + std::strlen(szUTF8String));
    }catch(...){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: failed to peek one utf8 code"));
    }
}

std::vector<size_t> UTF8Func::BuildOff(const char *szUTF8String)
{
    if(!szUTF8String){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    auto nStrLen = std::strlen(szUTF8String);
    if(nStrLen == 0){
        return {};
    }

    std::vector<size_t> stvOff;

    const char *pszCurr = szUTF8String;
    const char *pszEnd  = szUTF8String + nStrLen;

    stvOff.reserve(nStrLen);
    for(; pszCurr < pszEnd; utf8::advance(pszCurr, 1, pszEnd)){
        stvOff.push_back(pszCurr - szUTF8String);
    }

    return stvOff;
}
