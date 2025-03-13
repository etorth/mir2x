#pragma once
#include <cmath>
#include "totype.hpp"

enum AscendStrType: int
{
    ASCENDSTR_NONE  = 0,
    ASCENDSTR_MISS  = 0,
    ASCENDSTR_BEGIN = 1,
    ASCENDSTR_RED   = 1,
    ASCENDSTR_BLUE  = 2,
    ASCENDSTR_GREEN = 3,
    ASCENDSTR_END   = 4,
};

class AscendStr
{
    private:
        int m_type;
        int m_value;

    private:
        int m_x;
        int m_y;

    private:
        double m_tick = 0.0;

    public:
        AscendStr(int, int, int, int);

    public:
        AscendStr(int x, int y)
            : AscendStr(ASCENDSTR_MISS, 0, x, y)
        {}

    public:
        ~AscendStr() = default;

    public:
        void draw(int, int);

    public:
        void update(double fUpdate)
        {
            m_tick += fUpdate;
        }

    private:
        int type () const { return m_type;  }
        int value() const { return m_value; }

    private:
        int x() const
        {
            return m_x + to_d(std::lround(ratio() * 50.0));
        }

        int y() const
        {
            return m_y - to_d(std::lround(ratio() * 50.0));
        }

    public:
        double ratio() const
        {
            return m_tick / 3000.0;
        }
};
