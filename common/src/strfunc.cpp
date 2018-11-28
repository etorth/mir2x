/*
 * =====================================================================================
 *
 *       Filename: strfunc.cpp
 *        Created: 11/27/2018 22:36:12
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

#include <vector>
#include <cstring>
#include <stdexcept>
#include "strfunc.hpp"

bool str_nonempty(const char *szString)
{
    return szString && std::strlen(szString);
}

std::string str_vprintf(const char *szStrFormat, va_list ap)
{
    if(!szStrFormat){
        throw std::invalid_argument("str_vprintf(nullptr, ap)");
    }

    // 1. try static buffer
    //    give an enough size so we can hopefully stop here
    int nRetLen = -1;
    {

        // need to prepare this copy
        // in case parsing with static buffer failed

        va_list ap_static;
        va_copy(ap_static, ap);

        char szSBuf[256];
        nRetLen = std::vsnprintf(szSBuf, std::extent<decltype(szSBuf)>::value, szStrFormat, ap_static);
        va_end(ap_static);

        if(nRetLen >= 0){
            if((size_t)(nRetLen + 1) <= std::extent<decltype(szSBuf)>::value){
                return std::string(szSBuf);
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            throw std::runtime_error(((std::string("std::vsnprintf(\"") + szStrFormat) + "\") returns ") + std::to_string(nRetLen));
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory

    std::vector<char> szDBuf(nRetLen + 1 + 128);
    while(true){
        va_list ap_dynamic;
        va_copy(ap_dynamic, ap);

        nRetLen = std::vsnprintf(szDBuf.data(), szDBuf.size(), szStrFormat, ap_dynamic);
        va_end(ap_dynamic);

        if(nRetLen >= 0){
            if((size_t)(nRetLen + 1) <= szDBuf.size()){
                return std::string(szDBuf.data());
            }else{
                szDBuf.resize(nRetLen + 1 + 128);
            }
        }else{
            throw std::runtime_error(((std::string("std::vsnprintf(\"") + szStrFormat) + "\") returns ") + std::to_string(nRetLen));
        }
    }

    throw std::runtime_error((std::string("std::vsnprintf(\"") + szStrFormat) + "\", ap) reaches impossible point");
}

std::string str_printf(const char *szStrFormat, ...)
{
    va_list ap;
    va_start(ap, szStrFormat);

    try{
        auto stRetStr = str_vprintf(szStrFormat, ap);
        va_end(ap);
        return stRetStr;
    }catch(...){
        va_end(ap);
        throw;
    }
}
