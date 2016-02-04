/*
 * =====================================================================================
 *
 *       Filename: mathfunc.hpp
 *        Created: 02/02/2016 20:50:30
 *  Last Modified: 02/03/2016 21:09:54
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
#include <type_traits>

template<typename T> bool PointInSegment(T nfX, T nfStartX, T nfW)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return nfX >= nfStartX && nfX < nfStartX + nfW;
}

template<typename T> bool PointInRect(T nfX, T nfY, T nStartX, T nStartY, T nfW, T nfH)
{
    return PointInSegment(nfX, nStartX, nfW) && PointInSegment(nfY, nStartY, nfH);
}

template<typename T> bool PointInTriangle(T nfX, T nfY, T nfX1, T nfY1, T nfX2, T nfY2, T nfX3, T nfY3)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");

    auto bSign = [](T p1X, T p1Y, T p2X, T p2Y, T p3X, T p3Y){
        return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
    };

    bool b1 = bSign(nfX, nfY, nfX1, nfY1, nfX2, nfY2) <= 0;
    bool b2 = bSign(nfX, nfY, nfX2, nfY2, nfX3, nfY3) <= 0;
    bool b3 = bSign(nfX, nfY, nfX3, nfY3, nfX1, nfY1) <= 0;

    return ((b1 == b2) && (b2 == b3));
}

template<typename T> bool PointInCircle(T nfX, T nfY, T nfCX, T nfCY, T nfR)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    T dX = nfX - nfCX;
    T dY = nfY - nfCY;
    return dX * dX + dY * dY <= nfR * nfR;
}
