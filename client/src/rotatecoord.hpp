#pragma once

class RotateCoord
{
    public:
        RotateCoord();
        ~RotateCoord();
    public:
        bool Set(int, int, int, int, int, int);
        int  X();
        int  Y();
        bool MoveToNext();
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
    public:
       //      2
       //  <-------A
       //  |       |
       // 3|       |1
       //  |   0   |
       //  v------->
        int  m_Direction;
};
