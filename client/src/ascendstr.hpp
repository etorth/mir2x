/*
 * =====================================================================================
 *
 *       Filename: ascendstr.hpp
 *        Created: 07/20/2017 00:31:01
 *  Last Modified: 07/20/2017 18:54:32
 *
 *    Description: decide to not implement it as magic
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

#pragma once
enum AscendStrType: int
{
    ASCENDSTR_MISS = 0,
    ASCENDSTR_NUM0 = 1,
    ASCENDSTR_NUM1 = 2,
    ASCENDSTR_NUM2 = 3,
};

class AscendStr
{
    private:
        int m_Type;
        int m_Value;

    private:
        int m_X;
        int m_Y;

    private:
        double m_Tick;

    public:
        AscendStr(int, int, int, int);

    public:
        ~AscendStr() = default;

    public:
        void Draw(int, int);
        void Update(double);

    private:
        int Type () const { return m_Type;  }
        int Value() const { return m_Value; }

    private:
        int X() const
        {
            return m_X;
        }

        int Y() const
        {
            return m_Y - (int)(std::lround(Ratio() * 50.0));
        }

    private:
        double Tick() const
        {
            return m_Tick;
        }

    public:
        double Ratio() const
        {
            return Tick() / 3000.0;
        }
};
