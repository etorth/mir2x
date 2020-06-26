/*
 * =====================================================================================
 *
 *       Filename: toll.hpp
 *        Created: 03/07/2020 17:07:46
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
#include <string>

#define to_lld(x) static_cast<long long>(x)
#define to_llu(x) static_cast<unsigned long long>(x)

inline const char * to_cstr(const char *s)
{
    if(s == nullptr){
        return "(null)";
    }
    else if(s[0] == '\0'){
        return "(empty)";
    }
    else{
        return s;
    }
}

inline const char *to_cstr(const std::string &s)
{
    return to_cstr(s.c_str());
}

inline const char *to_boolcstr(bool b)
{
    return b ? "true" : "false";
}
