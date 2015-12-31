#pragma once
#include <array>
#include <utility>

class DirectiveRectCover
{
    public:
        DirectiveRectCover(int, double, double, double, double);
        ~DirectiveRectCover();

    public:
        const std::pair<double, double> &MidPoint() const;
        const std::pair<double, double> &Point(int) const;

    private:
        std::pair<double, double>                m_MidPoint;
        std::array<std::pair<double, double>, 4> m_PointV;

    private:
        int    m_Direction;
        double m_W;
        double m_H;
        double m_MinX;
        double m_MinY;
        double m_MaxX;
        double m_MaxY;

    public:
        int Direction() const;

    public:
        double W() const;
        double H() const;
        double MinX() const;
        double MinY() const;
        double MaxX() const;
        double MaxY() const;

    public:
        bool Overlap(const DirectiveRectCover &) const;

    private:
        void CalculatePointInfo();

    public:
        void DSet(double, double, double, double);
        void DMove(double, double);
        bool In(double, double) const;
};
