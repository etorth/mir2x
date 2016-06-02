#pragma once

#include <utility>
#include "mathfunc.hpp"

class RectCover
{
    private:
        double m_X1;
        double m_Y1;
        double m_X2;
        double m_Y2;
        double m_MidX;
        double m_MidY;
    public:
        RectCover();
        RectCover(double, double, double, double, double, double);
    public:
        void Set(double, double, double, double, double, double);
    public:
        double W();
        double H();
        double MidX();
        double MidY();
    public:
        std::pair<double, double> Point(int);
    public:
        bool Overlap(const RectCover &);
    public:
        void Move(double, double);
        bool In(double, double);
};
