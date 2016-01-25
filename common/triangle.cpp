#include "triangle.hpp"

Triangle::Triangle(
        double fX0, double fY0,
        double fX1, double fY1,
        double fX2, double fY2)
    // : m_PointV({{fX0, fY0}, {fX1, fY1}, {fX2, fY2}})
{
	m_PointV[0] = { fX0, fY0 };
	m_PointV[1] = { fX1, fY1 };
	m_PointV[2] = { fX2, fY2 };
    CalcalatePointInfo();
}

Triangle::~Triangle()
{}

void Triangle::CalcalatePointInfo()
{
    m_MaxX = (std::max)((std::max)(m_PointV[0].first,  m_PointV[1].first ), m_PointV[2].first );
    m_MaxY = (std::max)((std::max)(m_PointV[0].second, m_PointV[1].second), m_PointV[2].second);
    m_MinX = (std::min)((std::min)(m_PointV[0].first,  m_PointV[1].first ), m_PointV[2].first );
    m_MinY = (std::min)((std::min)(m_PointV[0].second, m_PointV[1].second), m_PointV[2].second);
}

bool Triangle::In(double fX, double fY) const
{
    if(m_MinX <= fX && fX <= m_MaxX && m_MinY <= fY && fY <= m_MaxY){
        auto bSign = [](double p1X, double p1Y, double p2X, double p2Y, double p3X, double p3Y){
            return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
        };

        bool b1 = bSign(fX, fY, m_PointV[0].first, m_PointV[0].second, m_PointV[1].first, m_PointV[1].second) <= 0.0f;
        bool b2 = bSign(fX, fY, m_PointV[1].first, m_PointV[1].second, m_PointV[2].first, m_PointV[2].second) <= 0.0f;
        bool b3 = bSign(fX, fY, m_PointV[2].first, m_PointV[2].second, m_PointV[0].first, m_PointV[0].second) <= 0.0f;

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
    for(auto &stPoint: m_PointV){
        stPoint.first  += fDX;
        stPoint.second += fDY;
    }
    m_MinX += fDX;
    m_MinY += fDY;
    m_MaxX += fDX;
    m_MaxY += fDY;
}

double Triangle::MaxX() const
{
    return m_MaxX;
}

double Triangle::MaxY() const
{
    return m_MaxY;
}

double Triangle::MinX() const
{
    return m_MinX;
}

double Triangle::MinY() const
{
    return m_MinY;
}

const std::pair<double, double> &Triangle::Point(int nIndex) const
{
    return m_PointV[nIndex % 3];
}
