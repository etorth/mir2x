/*
 * =====================================================================================
 *
 *       Filename: rotatecoord.hpp
 *        Created: 08/15/2015 04:01:57
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

class RotateCoord
{
    private:
        // define the direction current x/y increments
        //      2
        //  <-------A
        //  |       |
        // 3|       |1
        //  |   0   |
        //  v------->
        //
        enum class DirType
        {
            DIR_0 = 0,
            DIR_1,
            DIR_2,
            DIR_3,
        };

    private:
        const int m_centerX;
        const int m_centerY;

    private:
        const int m_startX;
        const int m_startY;
        const int m_stopX;
        const int m_stopY;

    private:
        int m_distance;

    private:
        DirType m_direction;

    private:
        int m_currentX;
        int m_currentY;

    private:
        bool m_overlap[4];

    public:
        RotateCoord(int, int, int, int, int, int);

    public:
        ~RotateCoord() = default;

    public:
        int X() const
        {
            return m_currentX;
        }

        int Y() const
        {
            return m_currentY;
        }

    public:
        bool forward();

    private:
        void CheckOverlap();
        bool MoveToNextRound();
};
