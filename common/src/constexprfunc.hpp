/*
 * =====================================================================================
 *
 *       Filename: constexprfunc.hpp
 *        Created: 08/05/2017 12:14:11
 *  Last Modified: 08/06/2017 15:12:15
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

    template<typename... U> constexpr bool CheckIntParam(int nCheckInt, int nValidInt, U&&... u)
    {
        return (nCheckInt == nValidInt) || CheckIntParam(nCheckInt, std::forward<U>(u)...);
    }

    template<> constexpr bool CheckIntParam(int nCheckInt, int nValidInt)
    {
        return nCheckInt == nValidInt;
    }
}
