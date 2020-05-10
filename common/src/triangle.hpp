#pragma once
#include <array>
#include <utility>

class Triangle final
{
    // ---------------->

    // P1              P2
    // +---------------+
    // |\             /|
    // | \           / |
    // |  \         /  |
    // |   \       /   |
    // |    \     /    |
    // |     \   /     |
    // |      \ /      |
    // |       V       |
    // |      P0       |

    public:
        Triangle(double, double, double, double, double, double);
        ~Triangle();

    public:
        const std::pair<double, double> &Point(int) const;

    public:
        bool Overlap(const Triangle &) const;
        bool In(double, double) const;

    public:
        void DMove(double, double);

    private:
        double m_minX;
        double m_minY;
        double m_maxX;
        double m_maxY;

    public:
        double MinX() const;
        double MinY() const;
        double MaxX() const;
        double MaxY() const;

    private:
        void CalcalatePointInfo();
        bool StrongCheckOverlap(const Triangle &);

    private:
        std::array<std::pair<double, double>, 3> m_pointV;
};
