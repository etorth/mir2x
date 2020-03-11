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
        const int m_CenterX;
        const int m_CenterY;

    private:
        const int m_StartX;
        const int m_StartY;
        const int m_StopX;
        const int m_StopY;

    private:
        int m_Distance;

    private:
        DirType m_Direction;

    private:
        int m_CurrentX;
        int m_CurrentY;

    private:
        bool m_Overlap[4];

    public:
        RotateCoord(int, int, int, int, int, int);

    public:
        ~RotateCoord() = default;

    public:
        int X() const
        {
            return m_CurrentX;
        }

        int Y() const
        {
            return m_CurrentY;
        }

    public:
        bool Forward();

    private:
        void CheckOverlap();
        bool MoveToNextRound();
};
