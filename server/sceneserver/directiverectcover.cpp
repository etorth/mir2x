/*
 * =====================================================================================
 *
 *       Filename: directiverectcover.cpp
 *        Created: 8/7/2015 2:36:13 AM
 *  Last Modified: 09/02/2015 4:56:13 AM
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

#include "triangle.hpp"
#include "directiverectcover.hpp"
#include <limits>
#include <utility>

DirectiveRectCover::DirectiveRectCover(int nDir, 
        double fMidX, double fMidY, double fW, double fH)
    : m_Direction(nDir)
    , m_MidPoint({fMidX, fMidY})
    , m_W((std::max)(0.0, fW))
    , m_H((std::max)(0.0, fH))
{
    CalculatePointInfo();
}

DirectiveRectCover::~DirectiveRectCover()
{}

void DirectiveRectCover::CalculatePointInfo()
{
    double fX1   = 0.0;
    double fX2   = 0.0;
    double fY1   = 0.0;
    double fY2   = 0.0;
    double fW    = m_W;
    double fH    = m_H;
    double fMidX = m_MidPoint.first;
    double fMidY = m_MidPoint.second;

    switch(m_Direction){
        case 0:
            {
                fX1 = fMidX - fW / 2.0;
                fY1 = fMidY - fH / 2.0;
                fX2 = fMidX + fW / 2.0;
                fY2 = fMidY - fH / 2.0;
                break;
            }
        case 2:
            {
                fX1 = fMidX + fH / 2.0;
                fY1 = fMidY - fW / 2.0;
                fX2 = fMidX + fH / 2.0;
                fY2 = fMidY + fW / 2.0;
                break;
            }
        case 4:
            {
                fX1 = fMidX + fW / 2.0;
                fY1 = fMidY + fH / 2.0;
                fX2 = fMidX - fW / 2.0;
                fY2 = fMidY + fH / 2.0;
                break;
            }
        case 6:
            {
                fX1 = fMidX - fH / 2.0;
                fY1 = fMidY + fW / 2.0;
                fX2 = fMidX - fH / 2.0;
                fY2 = fMidY - fW / 2.0;
                break;
            }
        case 1:
            {
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;

                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1 = fTX1;
                fY1 = fTY1;
                fX2 = fTX2;
                fY2 = fTY2;

                break;
            }
        case 3:
            {
                std::swap(fW, fH);
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;

                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = fTX2;
                fY1   = fTY2;
                fX2   = 2.0 * fMidX - fTX1;
                fY2   = 2.0 * fMidY - fTY1;

                break;
            }
        case 5:
            {
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;
                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = 2.0 * fMidX - fTX1;
                fY1   = 2.0 * fMidY - fTY1;
                fX2   = 2.0 * fMidX - fTX2;
                fY2   = 2.0 * fMidY - fTY2;

                break;
            }
        case 7:
            {
                std::swap(fW, fH);
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;
                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = 2.0 * fMidX - fTX2;
                fY1   = 2.0 * fMidY - fTY2;
                fX2   = fTX1;
                fY2   = fTY1;

                break;
            }
        default:
            break;
    }

    double fX3 = 2.0 * fMidX - fX1;
    double fY3 = 2.0 * fMidY - fY1;
    double fX4 = 2.0 * fMidX - fX2;
    double fY4 = 2.0 * fMidY - fY2;

    // m_PointV = {{fX1, fY1}, {fX2, fY2}, {fX3, fY3}, {fX4, fY4}};
    m_PointV[0] = {fX1, fY1};
    m_PointV[1] = {fX2, fY2};
    m_PointV[2] = {fX3, fY3};
    m_PointV[3] = {fX4, fY4};


    m_MaxX   = (std::max)((std::max)((std::max)(fX1, fX2), fX3), fX4);
    m_MaxY   = (std::max)((std::max)((std::max)(fY1, fY2), fY3), fY4);
    m_MinX   = (std::min)((std::min)((std::min)(fX1, fX2), fX3), fX4);
    m_MinY   = (std::min)((std::min)((std::min)(fY1, fY2), fY3), fY4);
}

const std::pair<double, double> &DirectiveRectCover::Point(int nIndex) const
{
    return m_PointV[nIndex % 4];
}

double DirectiveRectCover::MaxX() const
{
    return m_MaxX;
}

double DirectiveRectCover::MaxY() const
{
    return m_MaxY;
}

double DirectiveRectCover::MinX() const
{
    return m_MinX;
}

double DirectiveRectCover::MinY() const
{
    return m_MinY;
}

bool DirectiveRectCover::Overlap(const DirectiveRectCover &stRect) const
{
    if(true
        && MinX() <= stRect.MaxX()
        && MaxX() >= stRect.MinX()
        && MinY() <= stRect.MaxY()
        && MaxY() >= stRect.MinY()
      ){
        const DirectiveRectCover *pRect1 = this;
        const DirectiveRectCover *pRect2 = &stRect;
        for(int nCnt = 0; nCnt < 4; ++nCnt){
            if(pRect1->In(pRect2->Point(nCnt).first, pRect2->Point(nCnt).second)){
                return true;
            }
        }
        std::swap(pRect1, pRect2);
        for(int nCnt = 0; nCnt < 4; ++nCnt){
            if(pRect1->In(pRect2->Point(nCnt).first, pRect2->Point(nCnt).second)){
                return true;
            }
        }
    }
    return false;
}

// W is just distance from Point1 to Point2, not real width
double DirectiveRectCover::W() const
{
    return m_W;
}

// H is just distance from Point2 to Point3, not real width
double DirectiveRectCover::H() const
{
    return m_H;
}

const std::pair<double, double> &DirectiveRectCover::MidPoint() const
{
    return m_MidPoint;
}

void DirectiveRectCover::DMove(double fDX, double fDY)
{
    m_MidPoint.first  += fDX;
    m_MidPoint.second += fDY;

    m_PointV[0].first  += fDX;
    m_PointV[0].second += fDX;
    m_PointV[1].first  += fDX;
    m_PointV[1].second += fDX;
    m_PointV[2].first  += fDX;
    m_PointV[2].second += fDX;
    m_PointV[3].first  += fDX;
    m_PointV[3].second += fDX;
}

void DirectiveRectCover::DSet(double fDX, double fDY, double fDW, double fDH)
{
    // no way to reset direction, it makes no sense
    m_MidPoint.first  += fDX;
    m_MidPoint.second += fDY;

    m_W = (std::max)(0.0, m_W + fDW);
    m_H = (std::max)(0.0, m_H + fDH);

    CalculatePointInfo();
}

bool DirectiveRectCover::In(double fX, double fY) const
{
    Triangle stTri1(
            m_PointV[0].first, m_PointV[0].second,
            m_PointV[1].first, m_PointV[1].second,
            m_PointV[2].first, m_PointV[2].second);
    Triangle stTri2(
            m_PointV[0].first, m_PointV[0].second,
            m_PointV[3].first, m_PointV[3].second,
            m_PointV[2].first, m_PointV[2].second);

    return stTri1.In(fX, fY) || stTri1.In(fX, fY);
}

int DirectiveRectCover::Direction() const
{
    return m_Direction;
}
