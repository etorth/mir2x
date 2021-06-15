/*
 * =====================================================================================
 *
 *       Filename: mathf.hpp
 *        Created: 02/02/2016 20:50:30
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
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

namespace mathf
{
    // used for ability calculation
    // accepts 10-0 which always return 10
    inline int rand(int min, int max)
    {
        return min + std::rand() % (1 + std::max<int>(max - min, 0));
    }

    template<typename T> T bound(T val, T min, T max)
    {
        return std::min<T>(std::max<T>(val, min), max);
    }

    template<typename T> T CDistance(T nfX, T nfY, T nfX1, T nfY1)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        if(std::is_unsigned<T>::value){
            // for unsigned intergal and bool
            if(nfX < nfX1){
                std::swap(nfX, nfX1);
            }

            if(nfY < nfY1){
                std::swap(nfY, nfY1);
            }
        }
        return (std::max<T>)(std::abs(nfX - nfX1), std::abs(nfY - nfY1));
    }

    template<typename T> T LDistance2(T nfX, T nfY, T nfX1, T nfY1)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        if(std::is_unsigned<T>::value){
            // for unsigned intergal and bool
            if(nfX < nfX1){
                std::swap(nfX, nfX1);
            }

            if(nfY < nfY1){
                std::swap(nfY, nfY1);
            }
        }
        return (nfX - nfX1) * (nfX - nfX1) + (nfY - nfY1) * (nfY - nfY1);
    }

    template<typename T> T LDistance(T nfX, T nfY, T nfX1, T nfY1)
    {
        return T(std::sqrt(LDistance2(nfX, nfY, nfX1, nfY1)));
    }

    template<typename T> bool circleOverlap(T nfX, T nfY, T nfR, T nfX1, T nfY1, T nfR1)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        return LDistance2(nfX, nfY, nfX1, nfY1) <= (nfR + nfR1) * (nfR + nfR1);
    }

    template<typename T> bool circleRectangleOverlap(T nfCX, T nfCY, T nfCR, T nfX, T nfY, T nfW, T nfH)
    {
        // there may be one pixel problem, keep it in mind
        T nfRCX = nfX + nfW / 2;
        T nfRCY = nfY + nfH / 2;

        // be careful for the unsigned issue
        // std::abs(a - b) won't give you correct result if a < b and a, b are unisgned
        T nfDX = (nfRCX >= nfCX) ? (nfRCX - nfCX) : (nfCX - nfRCX);
        T nfDY = (nfRCY >= nfCY) ? (nfRCY - nfCY) : (nfCY - nfRCY);

        if(nfDX > (nfW / 2 + nfCR)){
            return false;
        }

        if(nfDY > (nfH / 2 + nfCR)){
            return false;
        }

        if(nfDX <= (nfW / 2)){
            return true;
        } 

        if(nfDY <= (nfH / 2)){
            return true;
        }

        return LDistance2(nfDX, nfDY, nfW / 2, nfH / 2) <= nfCR * nfCR;
    }

    template<typename T> bool rectangleCircleOverlap(T nfX, T nfY, T nfW, T nfH, T nfCX, T nfCY, T nfCR)
    {
        return circleRectangleOverlap(nfCX, nfCY, nfCR, nfX, nfY, nfW, nfH);
    }

    template<typename T> bool pointInSegment(T nfX, T nfStartX, T nfW)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        return nfX >= nfStartX && nfX < nfStartX + nfW;
    }

    template<typename T> bool pointInRectangle(T nfX, T nfY, T nStartX, T nStartY, T nfW, T nfH)
    {
        return pointInSegment(nfX, nStartX, nfW) && pointInSegment(nfY, nStartY, nfH);
    }

    template<typename T> bool pointInTriangle(T nfX, T nfY, T nfX1, T nfY1, T nfX2, T nfY2, T nfX3, T nfY3)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");

        bool b1 = false;
        bool b2 = false;
        bool b3 = false;

        if(std::is_unsigned<T>::value){
            // for unsigned we have to conver it to double since we use substraction
            auto bSign = [](double p1X, double p1Y, double p2X, double p2Y, double p3X, double p3Y) -> double
            {
                return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
            };

            b1 = (bSign(nfX, nfY, nfX1, nfY1, nfX2, nfY2) <= 0.0);
            b2 = (bSign(nfX, nfY, nfX2, nfY2, nfX3, nfY3) <= 0.0);
            b3 = (bSign(nfX, nfY, nfX3, nfY3, nfX1, nfY1) <= 0.0);
        }else{
            auto bSign = [](T p1X, T p1Y, T p2X, T p2Y, T p3X, T p3Y) -> T{
                return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
            };

            b1 = (bSign(nfX, nfY, nfX1, nfY1, nfX2, nfY2) <= (T)(0));
            b2 = (bSign(nfX, nfY, nfX2, nfY2, nfX3, nfY3) <= (T)(0));
            b3 = (bSign(nfX, nfY, nfX3, nfY3, nfX1, nfY1) <= (T)(0));
        }

        return ((b1 == b2) && (b2 == b3));
    }

    template<typename T> bool pointInCircle(T nfX, T nfY, T nfCX, T nfCY, T nfR)
    {
        return LDistance2(nfX, nfY, nfCX, nfCY) <= nfR * nfR;
    }

    template<typename T> bool rectangleOverlap(T nfX1, T nfY1, T nfW1, T nfH1, T nfX2, T nfY2, T nfW2, T nfH2)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        return !((nfX1 >= nfX2 + nfW2) || (nfX1 + nfW1 < nfX2) || (nfY1 >= nfY2 + nfH2) || (nfY1 + nfH1 < nfY2));
    }

    // check whether R2 is inside R1
    template<typename T> bool rectangleInside(T nfX1, T nfY1, T nfW1, T nfH1, T nfX2, T nfY2, T nfW2, T nfH2)
    {
        // TODO
        // finish code
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        return true
            && (nfX1 <= nfX2) && (nfX1 + nfW1 >= nfX2 + nfW2)
            && (nfY1 <= nfY2) && (nfY1 + nfH1 >= nfY2 + nfH2);
    }

    template<typename T> bool intervalOverlap(T nfX1, T nfW1, T nfX2, T nfW2)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        // TODO
        // assume T2 is smaller
        return !(nfX1 + nfW1 <= nfX2 || nfX2 + nfW2 <= nfX1);
    }

    template<typename T> bool intervalOverlapRegion(T nfX1, T nfW1, T *nfX2, T *nfW2)
    {
        static_assert(std::is_arithmetic<T>::value);
        if(!(nfX2 && nfW2)){
            throw fflerror("invalid arguments");
        }

        if(!intervalOverlap(nfX1, nfW1, *nfX2, *nfW2)){
            return false;
        }

        const T nfX2End = *nfX2 + *nfW2;
        *nfX2 = std::max<T>(nfX1, *nfX2);
        *nfW2 = std::min<T>(nfX1 + nfW1, nfX2End) - *nfX2;
        return true;
    }

    template<typename T> bool rectangleOverlapRegion(T nfX1, T nfY1, T nfW1, T nfH1, T &nfX2, T &nfY2, T &nfW2, T &nfH2)
    {
        if(!rectangleOverlap(nfX1, nfY1, nfW1, nfH1, nfX2, nfY2, nfW2, nfH2)){
            return false;
        }

        const T nfRX = std::max<T>(nfX1, nfX2);
        const T nfRY = std::max<T>(nfY1, nfY2);
        const T nfRW = std::min<T>(nfX1 + nfW1, nfX2 + nfW2) - nfRX;
        const T nfRH = std::min<T>(nfY1 + nfH1, nfY2 + nfH2) - nfRY;

        nfX2 = nfRX;
        nfY2 = nfRY;
        nfW2 = nfRW;
        nfH2 = nfRH;
        return true;
    }

    template<typename T> bool powerOf2(T nParam)
    {
        static_assert(std::is_integral<T>::value, "Integral type required...");
        return !(nParam & (nParam - 1));
    }

    // TODO & TBD
    // this is portable but not optimized, gcc has builtin functions
    // as __builtin_clz(), google it, however it's not portable
    template<typename T> T roundByPowerOf2(T nParam)
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
    template<typename T> bool circleLineOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1)
    {
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");
        if(false
                || pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
                || pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)){ return true; }

        if(std::is_unsigned<T>::value){
            auto nfNX0 = (double)(nfX0) - (double)(nfCX);
            auto nfNX1 = (double)(nfX1) - (double)(nfCX);
            auto nfNY0 = (double)(nfY0) - (double)(nfCX);
            auto nfNY1 = (double)(nfY1) - (double)(nfCX);

            auto nfDD = nfNX0 * nfNY1 - nfNX1 * nfNY0;
            return nfCR * nfCR * LDistance2(nfX0, nfY0, nfX1, nfY1) - nfDD * nfDD;
        }else{
            T nfNX0 = nfX0 - nfCX;
            T nfNX1 = nfX1 - nfCX;
            T nfNY0 = nfY0 - nfCX;
            T nfNY1 = nfY1 - nfCX;

            T nfDD = nfNX0 * nfNY1 - nfNX1 * nfNY0;
            return nfCR * nfCR * LDistance2(nfX0, nfY0, nfX1, nfY1) - nfDD * nfDD;
        }
    }

    // This function check whether the segment locates inside the circle, or intersects with the circle by one
    // or two points, if both intersecting points are not on the segment we return *false*. possibilities are
    // 1. one or two end points are in the circle
    // 2. or, there are one or two points one the segment overlapping with the circle
    //
    // TODO: 1. assume R is positive
    //       2. assume two points are distinct
    template<typename T> bool circleSegmentOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1)
    {
        // 1. check type
        static_assert(std::is_arithmetic<T>::value, "Arithmetic type required...");

        // 2. one or two end points are inside the circle
        if(false
                || pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
                || pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)){
            return true;
        }

        // 3. delta should be non-negative
        if(!(circleLineOverlap(nfCX, nfCY, nfCR, nfX0, nfY0, nfX1, nfY1))){
            return false;
        }

        // 4. f(t) = xt ^ 2 + yt ^ 2 - r^2, then
        //      a. f(0) >= 0 => !pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
        //      b. f(1) >= 0 => !pointInCircle(nfX1, nfY1, nfCX, nfCY, nfCR)
        //      c. 0 <= (-b / 2a) <= 1 
        //    here a, b are already ok, so only for c

        if(std::is_unsigned<T>::value){
            auto nfNX0 = (double)(nfX0) - (double)(nfCX);
            auto nfNX1 = (double)(nfX1) - (double)(nfCX);
            auto nfNY0 = (double)(nfY0) - (double)(nfCX);
            auto nfNY1 = (double)(nfY1) - (double)(nfCX);

            return true
                && (nfNX0 * nfNX0 - nfNX0 * nfNX1 + nfNY0 * nfNY0 - nfNY0 * nfNY1 >= 0.0)
                && (nfNX1 * nfNX1 - nfNX0 * nfNX1 + nfNY1 * nfNY1 - nfNY0 * nfNY1 >= 0.0);
        }else{
            T nfNX0 = nfX0 - nfCX;
            T nfNX1 = nfX1 - nfCX;
            T nfNY0 = nfY0 - nfCX;
            T nfNY1 = nfY1 - nfCX;

            return true
                && (nfNX0 * nfNX0 - nfNX0 * nfNX1 + nfNY0 * nfNY0 - nfNY0 * nfNY1 >= 0)
                && (nfNX1 * nfNX1 - nfNX0 * nfNX1 + nfNY1 * nfNY1 - nfNY0 * nfNY1 >= 0);
        }
    }

    // determine whether a circle is overlapped with a triangle
    // TODO 1. assume three distinct points to form an un-degraded triangle
    //      2. assume the R is positive
    //
    // algorithm from:
    // 1. http://stackoverflow.com/questions/540014/compute-the-area-of-intersection-between-a-circle-and-a-triangle
    // 2. http://mathworld.wolfram.com/Circle-LineIntersection.html
    //
    template<typename T> bool circleTriangleOverlap(T nfCX, T nfCY, T nfCR, T nfX0, T nfY0, T nfX1, T nfY1, T nfX2, T nfY2)
    {
        // 1. first check whether there is any point in the circle
        if(false
                || pointInCircle(nfX0, nfY0, nfCX, nfCY, nfCR)
                || pointInCircle(nfX1, nfY1, nfCX, nfCY, nfCR)
                || pointInCircle(nfX2, nfY2, nfCX, nfCY, nfCR)){
            return true;
        }

        // 2. ok there isn't any point in the circle, check whether we have edges intersect with the circle
        //    actually we can skip check-1 and do this but this is expensive so...
        if(false
                || circleSegmentOverlap(nfCX, nfCY, nfCR, nfX0, nfY0, nfX1, nfY1)
                || circleSegmentOverlap(nfCX, nfCY, nfCR, nfX1, nfY1, nfX2, nfY2)
                || circleSegmentOverlap(nfCX, nfCY, nfCR, nfX2, nfY2, nfX0, nfY0)){
            return true;
        }

        // 3. ok no point inside circle and no circle-edge intersection, two possibilities:
        //    3-a. circle is inside the triangle
        //    3-b. circle and triangle are distinct

        // for 3-a, based on condition of case-3, then get a equivlence
        // circle is in triangle <=> center point is in triangle

        return pointInTriangle(nfCX, nfCY, nfX0, nfY0, nfX1, nfY1, nfX2, nfY2);
    }

    inline bool locateLineSegment(int nX, int nY, int nW, int nH, int *pX1, int *pY1, int *pX2, int *pY2)
    {
        // Liang-Barsky clipping algorithm.
        // https://github.com/smcameron/liang-barsky-in-c

        if(true
                && nW > 0
                && nH > 0
                && pX1
                && pY1
                && pX2
                && pY2){

            int nDX = *pX2 - *pX1;
            int nDY = *pY2 - *pY1;

            if(true
                    && nDX == 0
                    && nDY == 0
                    && pointInRectangle<int>(*pX1, *pY1, nX, nY, nW, nH)){
                return true;
            }

            auto fnClipT = [](int nNum, int nDenom, double &ftE, double &ftL) -> bool
            {
                if(nDenom == 0){
                    return nNum <= 0;
                }else{
                    double fT = 1.0 * nNum / nDenom;
                    if(nDenom > 0){
                        if(fT > ftL){
                            return false;
                        }
                        if(fT > ftE){
                            ftE = fT;
                        }
                    }else{
                        if(fT < ftE){
                            return false;
                        }
                        if(fT < ftL){
                            ftL = fT;
                        }
                    }
                    return true;
                }
            };

            double ftE = 0.0;
            double ftL = 1.0;

            if(true
                    && fnClipT( (nX - *pX1),           nDX, ftE, ftL)
                    && fnClipT(-(nX - *pX1) - nW + 1, -nDX, ftE, ftL)
                    && fnClipT( (nY - *pY1),           nDY, ftE, ftL)
                    && fnClipT(-(nY - *pY1) - nH + 1, -nDY, ftE, ftL)){

                if(ftL < 1.0){
                    *pX2 = to_d(std::lround(*pX1 + ftL * nDX));
                    *pY2 = to_d(std::lround(*pY1 + ftL * nDY));
                }

                if(ftE > 0.0){
                    *pX1 += to_d(std::lround(ftE * nDX));
                    *pY1 += to_d(std::lround(ftE * nDY));
                }
                return true;
            }
        }
        return false;
    }

    inline bool ROICrop(
            int *pSrcX, int *pSrcY,  // the default parameters we used in Widget::drawEx
            int *pSrcW, int *pSrcH,  // ...
            int *pDstX, int *pDstY,  // ...

            int nSrcOriginW,         // original size of the source image
            int nSrcOriginH,         // ...

            int nSrcRegionX = 0, int nSrcRegionY = 0, int nSrcRegionW = -1, int nSrcRegionH = -1,  // ROI of source, by default uses fully
            int nDstRegionX = 0, int nDstRegionY = 0, int nDstRegionW = -1, int nDstRegionH = -1)  // ROI of canvas, by default uses fully
    {
        if(!(pSrcX && pSrcY && pSrcW && pSrcH && pDstX && pDstY && (nSrcOriginW >= 0) && (nSrcOriginH >= 0))){
            throw fflerror("invalid argument: ROPCrop(...)");
        }

        if(nSrcRegionW < 0){
            nSrcRegionW = nSrcOriginW - nSrcRegionX;
        }

        if(nSrcRegionH < 0){
            nSrcRegionH = nSrcOriginH - nSrcRegionY;
        }

        int nDst2SrcRegionX = nDstRegionX + (*pSrcX - *pDstX);
        int nDst2SrcRegionY = nDstRegionY + (*pSrcY - *pDstY);

        if(nDstRegionW < 0){
            nDstRegionW = nSrcOriginW - nDst2SrcRegionX;
        }

        if(nDstRegionH < 0){
            nDstRegionH = nSrcOriginH - nDst2SrcRegionY;
        }

        int nDst2SrcRegionW = nDstRegionW;
        int nDst2SrcRegionH = nDstRegionH;

        if(!rectangleOverlapRegion(nSrcRegionX, nSrcRegionY, nSrcRegionW, nSrcRegionH, nDst2SrcRegionX, nDst2SrcRegionY, nDst2SrcRegionW, nDst2SrcRegionH)){
            return false;
        }

        int nBkupSrcX = *pSrcX;
        int nBkupSrcY = *pSrcY;
        int nBkupSrcW = *pSrcW;
        int nBkupSrcH = *pSrcH;

        if(*pSrcW < 0){
            *pSrcW = nSrcOriginW - *pSrcX;
        }

        if(*pSrcH < 0){
            *pSrcH = nSrcOriginH - *pSrcY;
        }

        if(!rectangleOverlapRegion(nDst2SrcRegionX, nDst2SrcRegionY, nDst2SrcRegionW, nDst2SrcRegionH, *pSrcX, *pSrcY, *pSrcW, *pSrcH)){
            *pSrcW = nBkupSrcW;
            *pSrcH = nBkupSrcH;
            return false;
        }

        *pDstX += (*pSrcX - nBkupSrcX);
        *pDstY += (*pSrcY - nBkupSrcY);

        return true;
    }
}
