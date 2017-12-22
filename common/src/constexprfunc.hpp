/*
 * =====================================================================================
 *
 *       Filename: constexprfunc.hpp
 *        Created: 08/05/2017 12:14:11
 *  Last Modified: 08/08/2017 00:53:28
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

    // CheckIntMap(szStr, D_NONE,
    //              u8"攻击", D_ATTACK,
    //              u8"挨打", D_HITTEF,
    //              u8"行走", D_WALK)
    template<typename... U> constexpr int CheckIntMap(const char *szStr, int nDefaultInt, const char *szOptStr1, int nOptInt1, U&&... u)
    {
        return (CheckIntMap(szStr, nDefaultInt, szOptStr1, nOptInt1) == nOptInt1) ? nOptInt1 : CheckIntMap(szStr, nDefaultInt, std::forward<U>(u)...);
    }

    template<> constexpr int CheckIntMap(const char *szStr, int nDefaultInt, const char *szOptStr1, int nOptInt1)
    {
        return CompareUTF8(szStr, szOptStr1) ? nOptInt1 : nDefaultInt;
    }
}
