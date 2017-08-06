/*
 * =====================================================================================
 *
 *       Filename: constexprfunc.hpp
 *        Created: 08/05/2017 12:14:11
 *  Last Modified: 08/05/2017 12:17:38
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
namespace ConstExprFunc
{
    constexpr bool CompareUTF8(const char *szStr1, const char *szStr2)
    {
        if(szStr1 && szStr2){
            while(*szStr1 == *szStr2){
                if(*szStr1){
                    ++szStr1;
                    ++szStr2;
                }else{
                    return true;
                }
            }
        }
        return false;
    }
}
