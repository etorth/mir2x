/*
 * =====================================================================================
 *
 *       Filename: rectcover.cpp
 *        Created: 8/7/2015 2:36:13 AM
 *  Last Modified: 04/15/2016 22:29:26
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

#include <cmath>
#include "rectcover.hpp"
#include <limits>
#include <utility>
#include "mathfunc.hpp"

RectCover::RectCover()
    : m_X1(0.0)
    , m_Y1(0.0)
    , m_X2(0.0)
    , m_Y2(0.0)
    , m_MidX(0.0)
    , m_MidY(0.0)
{}

RectCover::RectCover(double fX1, double fY1, double fX2, double fY2, double fMidX, double fMidY)
    : m_X1(fX1)
    , m_Y1(fY1)
    , m_X2(fX2)
    , m_Y2(fY2)
    , m_MidX(fMidX)
    , m_MidY(fMidY)
{}

void RectCover::Set(double fX1, double fY1, double fX2, double fY2, double fMidX, double fMidY)
{
    m_X1   = fX1;
    m_Y1   = fY1;
    m_X2   = fX2;
    m_Y2   = fY2;
    m_MidX = fMidX;
    m_MidY = fMidY;
}

std::pair<double, double> RectCover::Point(int nIndex)
{
    nIndex %= 5;
    switch(nIndex){
        case 0:
            return std::make_pair(m_MidX, m_MidY);
            break;
        case 1:
            return std::make_pair(m_X1, m_Y1);
            break;
        case 2:
            return std::make_pair(m_X2, m_Y2);
            break;
        case 3:
            return std::make_pair(2.0 * m_MidX - m_X1, 2.0 * m_MidY - m_Y1);
            break;
        case 4:
            return std::make_pair(2.0 * m_MidX - m_X2, 2.0 * m_MidY - m_Y2);
            break;
        default:
            return std::make_pair<double, double>(
                    std::numeric_limits<double>::max(),
                    std::numeric_limits<double>::max());
            break;
    }
}

bool RectCover::Overlap(const RectCover &/* cRect */)
{
    return false;
}

// W is just distance from Point1 to Point2, not real width
double RectCover::W()
{
    double fDX   = (m_X1 - m_X2 ) * 1.0;
    double fDY   = (m_Y1 - m_Y2 ) * 1.0;
    return std::sqrt(fDX * fDX + fDY * fDY);
}

// H is just distance from Point2 to Point3, not real width
double RectCover::H()
{
    double fTMidX   = (m_X1 + m_X2 ) / 2.0;
    double fTMidY   = (m_Y1 + m_Y2 ) / 2.0;
    double fDX      = (m_MidX - fTMidX);
    double fDY      = (m_MidY - fTMidY);
    return std::sqrt(fDX * fDX + fDY * fDY) * 2;
}

double RectCover::MidX()
{
    return m_MidX;
}

double RectCover::MidY()
{
    return m_MidY;
}

void RectCover::Move(double fDX, double fDY)
{
    m_X1   += fDX;
    m_Y1   += fDY;
    m_X2   += fDX;
    m_Y2   += fDY;
    m_MidX += fDX;
    m_MidY += fDY;
}

bool RectCover::In(double fX, double fY)
{
    return false
        || PointInTriangle(fX, fY, m_MidX, m_MidY, m_X1, m_Y1, m_X2, m_Y2)
        || PointInTriangle(fX, fY, m_MidX, m_MidY, m_MidX * 2.0 - m_X1, m_MidY * 2.0 - m_Y1, m_X2, m_Y2)
        || PointInTriangle(fX, fY, m_MidX, m_MidY, m_MidX * 2.0 - m_X2, m_MidY * 2.0 - m_Y2, m_X1, m_Y1)
        || PointInTriangle(fX, fY, m_MidX, m_MidY, m_MidX * 2.0 - m_X1, m_MidY * 2.0 - m_Y1, m_MidX * 2.0 - m_X2, m_MidY * 2.0 - m_Y2)
        ;
}
