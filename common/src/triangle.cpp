#include "triangle.hpp"

Triangle::Triangle(
        double fX0, double fY0,
        double fX1, double fY1,
        double fX2, double fY2)
    // : m_pointV({{fX0, fY0}, {fX1, fY1}, {fX2, fY2}})
{
	m_pointV[0] = { fX0, fY0 };
	m_pointV[1] = { fX1, fY1 };
	m_pointV[2] = { fX2, fY2 };
    CalcalatePointInfo();
}

Triangle::~Triangle()
{}

void Triangle::CalcalatePointInfo()
{
    m_maxX = (std::max<double>)((std::max<double>)(m_pointV[0].first,  m_pointV[1].first ), m_pointV[2].first );
    m_maxY = (std::max<double>)((std::max<double>)(m_pointV[0].second, m_pointV[1].second), m_pointV[2].second);
    m_minX = (std::min<double>)((std::min<double>)(m_pointV[0].first,  m_pointV[1].first ), m_pointV[2].first );
    m_minY = (std::min<double>)((std::min<double>)(m_pointV[0].second, m_pointV[1].second), m_pointV[2].second);
}

bool Triangle::In(double fX, double fY) const
{
    if(m_minX <= fX && fX <= m_maxX && m_minY <= fY && fY <= m_maxY){
        auto bSign = [](double p1X, double p1Y, double p2X, double p2Y, double p3X, double p3Y){
            return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
        };

        bool b1 = bSign(fX, fY, m_pointV[0].first, m_pointV[0].second, m_pointV[1].first, m_pointV[1].second) <= 0.0f;
        bool b2 = bSign(fX, fY, m_pointV[1].first, m_pointV[1].second, m_pointV[2].first, m_pointV[2].second) <= 0.0f;
        bool b3 = bSign(fX, fY, m_pointV[2].first, m_pointV[2].second, m_pointV[0].first, m_pointV[0].second) <= 0.0f;

        return ((b1 == b2) && (b2 == b3));
    }
    return false;
}

bool Triangle::Overlap(const Triangle & stTri) const
{
    if(true
            && MinX() <= stTri.MaxX()
            && MaxX() >= stTri.MinX()
            && MinY() <= stTri.MaxY()
            && MaxY() >= stTri.MinY()
      ){
        const Triangle *pTri1 = this;
        const Triangle *pTri2 = &stTri;

        for(int nCnt = 0; nCnt < 3; ++nCnt){
            auto stPoint = pTri1->Point(nCnt);
            if(pTri2->In(stPoint.first, stPoint.second)){
                return true;
            }
        }

        std::swap(pTri1, pTri2);

        for(int nCnt = 0; nCnt < 3; ++nCnt){
            auto stPoint = pTri1->Point(nCnt);
            if(pTri2->In(stPoint.first, stPoint.second)){
                return true;
            }
        }
    }
    return false;
}

void Triangle::DMove(double fDX, double fDY)
{
    for(auto &stPoint: m_pointV){
        stPoint.first  += fDX;
        stPoint.second += fDY;
    }
    m_minX += fDX;
    m_minY += fDY;
    m_maxX += fDX;
    m_maxY += fDY;
}

double Triangle::MaxX() const
{
    return m_maxX;
}

double Triangle::MaxY() const
{
    return m_maxY;
}

double Triangle::MinX() const
{
    return m_minX;
}

double Triangle::MinY() const
{
    return m_minY;
}

const std::pair<double, double> &Triangle::Point(int nIndex) const
{
    return m_pointV[nIndex % 3];
}
