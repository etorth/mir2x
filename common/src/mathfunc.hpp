/*
 * =====================================================================================
 *
 *       Filename: mathfunc.hpp
 *        Created: 02/02/2016 20:50:30
 *  Last Modified: 06/18/2016 23:51:31
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
#include <cmath>
#include <cassert>
#include <algorithm>
#include <type_traits>

template<typename T> T LDistance2(T nfX, T nfY, T nfX1, T nfY1)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return (nfX - nfX1) * (nfX - nfX1) + (nfY - nfY1) * (nfY - nfY1);
}

template<typename T> T LDistance(T nfX, T nfY, T nfX1, T nfY1)
{
    return T(std::sqrt(LDistance2(nfX, nfY, nfX1, nfY1)));
}

template<typename T> bool CircleOverlap(T nfX, T nfY, T nfR, T nfX1, T nfY1, T nfR1)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return LDistance2(nfX, nfY, nfX1, nfY1) <= (nfR + nfR1) * (nfR + nfR1);
}

template<typename T> bool CircleRectangleOverlap(T nfCX, T nfCY, T nfCR, T nfX, T nfY, T nfW, T nfH)
{
    // there may be one pixel problem, keep it in mind
    T nfRCX = nfX + nfW / 2;
    T nfRCY = nfY + nfH / 2;

    T nfDX = std::abs(nfCX - nfRCX);
    T nfDY = std::abs(nfCY - nfRCY);

    if(nfDX > (nfW/2 + nfCR)){ return false; }
    if(nfDY > (nfH/2 + nfCR)){ return false; }

    if(nfDX <= (nfW/2)){ return true; } 
    if(nfDY <= (nfH/2)){ return true; }

    return (nfDX - nfW / 2) * (nfDX - nfW / 2) + (nfDY - nfH / 2) * (nfDY - nfH / 2) <= nfCR * nfCR;
}

template<typename T> bool RectangleCircleOverlap(T nfX, T nfY, T nfW, T nfH, T nfCX, T nfCY, T nfCR)
{
    return CircleRectangleOverlap(nfCX, nfCY, nfCR, nfX, nfY, nfW, nfH);
}

template<typename T> bool PointInSegment(T nfX, T nfStartX, T nfW)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return nfX >= nfStartX && nfX < nfStartX + nfW;
}

template<typename T> bool PointInRectangle(T nfX, T nfY, T nStartX, T nStartY, T nfW, T nfH)
{
    return PointInSegment(nfX, nStartX, nfW) && PointInSegment(nfY, nStartY, nfH);
}

template<typename T> bool PointInTriangle(T nfX, T nfY, T nfX1, T nfY1, T nfX2, T nfY2, T nfX3, T nfY3)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");

    auto bSign = [](T p1X, T p1Y, T p2X, T p2Y, T p3X, T p3Y) -> T{
        return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
    };

    bool b1 = (bSign(nfX, nfY, nfX1, nfY1, nfX2, nfY2) <= (T)(0));
    bool b2 = (bSign(nfX, nfY, nfX2, nfY2, nfX3, nfY3) <= (T)(0));
    bool b3 = (bSign(nfX, nfY, nfX3, nfY3, nfX1, nfY1) <= (T)(0));

    return ((b1 == b2) && (b2 == b3));
}

template<typename T> bool PointInCircle(T nfX, T nfY, T nfCX, T nfCY, T nfR)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    T dX = nfX - nfCX;
    T dY = nfY - nfCY;
    return dX * dX + dY * dY <= nfR * nfR;
}

template<typename T> bool RectangleOverlap(T nfX1, T nfY1, T nfW1, T nfH1, T nfX2, T nfY2, T nfW2, T nfH2)
{
    // TODO
    // finish code
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return !(true
            || nfX1 >= nfX2 + nfW2
            || nfY1 >= nfY2 + nfH2
            || nfX1 + nfW1 < nfX2
            || nfY1 + nfH1 < nfY2);
}

// check whether R2 is inside R1
template<typename T> bool RectangleInside(T nfX1, T nfY1, T nfW1, T nfH1, T nfX2, T nfY2, T nfW2, T nfH2)
{
    // TODO
    // finish code
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    return true
        && (nfX1 <= nfX2) && (nfX1 + nfW1 >= nfX2 + nfW2)
        && (nfY1 <= nfY2) && (nfY1 + nfH1 >= nfY2 + nfH2);
}

template<typename T> bool IntervalOverlap(T nfX1, T nfW1, T nfX2, T nfW2)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    // TODO
    // assume T2 is smaller
    return !(nfX1 + nfW1 <= nfX2 || nfX2 + nfW2 <= nfX1);
}

template<typename T> bool RectangleOverlapRegion(T nfX1, T nfY1, T nfW1, T nfH1, T *nfX2, T *nfY2, T *nfW2, T *nfH2)
{
    // 1. assertion, since we use them to provide arguments
    assert(nfX2 && nfY2 && nfW2 && nfH2);

    // 2. fast check for overlapping
    if(RectangleOverlap(nfX1, nfY1, nfW1, nfH1, *nfX2, *nfY2, *nfW2, *nfH2)){

        T nfRX, nfRY, nfRW, nfRH;
        // TODO
        //
        // we assume W, H are always non-negative

        nfRX = std::max(nfX1, *nfX2);
        nfRY = std::max(nfY1, *nfY2);
        nfRW = std::min(nfX1 + nfW1, *nfX2 + *nfW2) - nfRX;
        nfRH = std::min(nfY1 + nfH1, *nfY2 + *nfH2) - nfRY;

        *nfX2 = nfRX;
        *nfY2 = nfRY;
        *nfW2 = nfRW;
        *nfH2 = nfRH;

        return true;
    }

    // no overlap, just return false
    return false;
}

template<typename T> bool PowerOf2(T nParam)
{
    static_assert(std::is_integral<T>::value, "Integral type required...");
    return !(nParam & (nParam - 1));
}

// TODO & TBD
// this is portable but not optimized, gcc has builtin functions
// as __builtin_clz(), google it, however it's not portable
template<typename T> T RoundByPowerOf2(T nParam)
{
    static_assert(std::is_integral<T>::value, "Integral type required...");

    nParam |= (nParam >> 1);
    nParam |= (nParam >> 2);

    if(sizeof(T) >= 1){ nParam |= (nParam >>  4); }
    if(sizeof(T) >= 2){ nParam |= (nParam >>  8); }
    if(sizeof(T) >= 4){ nParam |= (nParam >> 16); }
    // if(sizeof(T) >= 8){ nParam |= (nParam >> 32); }

    return nParam + 1;
}

// This function checks whether the line formed by two provided points are intersecting with the circle, we ca
// only check the delta, but we firstly check the two end points to accelerate the procedure, if failed then we
// check the delta
//
// TODO: 1. R is positive
//       2. two points are distinct to form a line
template<typename T> bool CircleLineOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1)
{
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
    if(false
            || PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
            || PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)){ return true; }

    T nfDX = nfX1 - nfX0;
    T nfDY = nfY1 - nfY0;

    // I can't put assertion here since T could be double
    // assert(nfDX && nfDY);

    T nfNX0 = nfX0 - nfCX;
    T nfNX1 = nfX1 - nfCX;
    T nfNY0 = nfY0 - nfCX;
    T nfNY1 = nfY1 - nfCX;

    T nfDD = nfNX0 * nfNY1 - nfNX1 * nfNY0;

    return nfCR * nfCR * (nfDX * nfDX + nfDY * nfDY) - nfDD * nfDD;
}

// This function check whether the segment locates inside the circle, or intersects with the circle by one
// or two points, if both intersecting points are not on the segment we return *false*. possibilities are
// 1. one or two end points are in the circle
// 2. or, there are one or two points one the segment overlapping with the circle
//
// TODO: 1. assume R is positive
//       2. assume two points are distinct
template<typename T> bool CircleSegmentOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1)
{
    // 1. check type
    static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");

    // 2. one or two end points are inside the circle
    if(false
            || PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
            || PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)){ return true; }

    // 3. delta should be non-negative
    if(!(CircleLineOverlap(nfCX, nfCY, nfCR, nfX0, nfY0, nfX1, nfY1))){ return false; }

    // 4. f(t) = xt ^ 2 + yt ^ 2 - r^2, then
    //      a. f(0) >= 0 => !PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
    //      b. f(1) >= 0 => !PointInCircle(nfX1, nfY1, nfCX, nfCY, nfCR)
    //      c. 0 <= (-b / 2a) <= 1 
    //    here a, b are already ok, so only for c

    T nfNX0 = nfX0 - nfCX;
    T nfNX1 = nfX1 - nfCX;
    T nfNY0 = nfY0 - nfCX;
    T nfNY1 = nfY1 - nfCX;

    return true
        && (nfNX0 * nfNX0 - nfNX0 * nfNX1 + nfNY0 * nfNY0 - nfNY0 * nfNY1 >= 0)
        && (nfNX1 * nfNX1 - nfNX0 * nfNX1 + nfNY1 * nfNY1 - nfNY0 * nfNY1 >= 0);
}

// determine whether a circle is overlapped with a triangle
// TODO 1. assume three distinct points to form an un-degraded triangle
//      2. assume the R is positive
//
// algorithm is from:
// 1. http://stackoverflow.com/questions/540014/compute-the-area-of-intersection-between-a-circle-and-a-triangle
// 2. http://mathworld.wolfram.com/Circle-LineIntersection.html
//
template<typename T> bool CircleTriangleOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1, T nfX2, T nfY2)
{
    // 1. first check whether there is any point in the circle
    if(false
            || PointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
            || PointInCircle(nfX1, nfY1, nfCX, nfCY, nfCR)
            || PointInCircle(nfX2, nfY2, nfCX, nfCY, nfCR)){ return true; }

    // 2. ok there isn't any point in the circle, check whether we have edges intersect with the circle
    //    actually we can skip check-1 and do this but this is expensive so...
    if(false
            || CircleSegmentOverlap(nfCX, nfCY, nfCR, nfX0, nfY0, nfX1, nfY1)
            || CircleSegmentOverlap(nfCX, nfCY, nfCR, nfX1, nfY1, nfX2, nfY2)
            || CircleSegmentOverlap(nfCX, nfCY, nfCR, nfX2, nfY2, nfX0, nfY0)){ return true; }

    // 3. ok no point inside circle and no circle-edge intersection, two possibilities:
    //    3-a. circle is inside the triangle
    //    3-b. circle and triangle are distinct

    // for 3-a, based on condition of case-3, then get a equivlence
    // circle is in triangle <=> center point is in triangle

    return PointInTriangle(nfCX, nfCY, nfX0, nfY0, nfX1, nfY1, nfX2, nfY2);
}
