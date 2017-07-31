/*
 * =====================================================================================
 *
 *       Filename: rotatecoord.hpp
 *        Created: 08/15/2015 04:01:57
 *  Last Modified: 07/31/2017 11:53:12
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
#pragma once

class RotateCoord
{
    public:
        RotateCoord() = default;
       ~RotateCoord() = default;

    public:
        int X() const { return m_CurrentX; }
        int Y() const { return m_CurrentY; }

    public:
        bool Reset(int,
                int,
                int,
                int,
                int,
                int);
        bool Forward();

    private:
        void CheckOverlap();
        bool MoveToNextRound();

    private:
        bool m_Overlap[4];
        int  m_CurrentX;
        int  m_CurrentY;
        int  m_CenterX;
        int  m_CenterY;
        int  m_StartX;
        int  m_StartY;
        int  m_StopX;
        int  m_StopY;
        int  m_Distance;

    private:
        //      2
        //  <-------A
        //  |       |
        // 3|       |1
        //  |   0   |
        //  v------->
        int  m_Direction;
};
