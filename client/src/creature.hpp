/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/03/2016 00:16:44
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

#include <array>
#include <vector>

class Creature
{
    protected:
        uint32_t m_UID;
        uint32_t m_AddTime;

        int m_R;
        int m_X;
        int m_Y;
        uint32_t m_MapID;

    protected:
        int m_MotionState;

    protected:
        double m_FrameUpdateDelay;

    protected:
        int    m_Frame;
        int    m_Direction;
        int    m_State;
        int    m_NextState;
        double m_UpdateTime;
        int    m_Speed;

    private:
        bool    m_RedPoision;
        bool    m_GreenPoision;
        bool    m_StonePoision;

        bool    m_WhiteBody;
        bool    m_Holy;

    private:
        int     m_EstimateNextX;
        int     m_EstimateNextY;

        int     m_TargetX;
        int     m_targetY;

    protected:
        bool m_TypeV[256];

    public:
        Creature(uint32_t, uint32_t);
        virtual ~Creature();

    public:
        void ResetR(uint32_t nR)
        {
            m_R = nR;
        }

        void ResetMotionState(uint8_t nMotionState)
        {
            m_MotionState = nMotionState;
        }

        void ResetLocation(uint32_t nMapID, int nX, int nY)
        {
            m_MapID = nMapID;
            m_X = nX;
            m_Y = nY;
        }

    public:
        int X()
        {
            return m_X;
        }

        int Y()
        {
            return m_Y;
        }

    public:

        int EstimateNextX()
        {
            return m_EstimateNextX;
        }

        int EstimateNextY()
        {
            return m_EstimateNextY;
        }

    public:
        bool TryStepMove(int);

    public:
        int UID()
        {
            return m_UID;
        }

        int AddTime()
        {
            return m_AddTime;
        }

    public:
        bool Type(uint8_t nType)
        {
            return m_TypeV[nType];
        }

        void ResetType(uint8_t nType, bool bThisType)
        {
            m_TypeV[nType] = bThisType;
        }

    public:
        int  CalculateDirection(int, int);

        // update the next possible position based on current state
        void EstimateNextPosition(int);

    public:
        virtual uint32_t FrameCount() = 0;

    public:
        virtual void Draw() = 0;
        virtual void Update() = 0;

    public:
        void UpdateCurrentState();
        void UpdateWithNewState();

    protected:
        void UpdateMotion(int);

    public:
        virtual void SetNextState(int);
        virtual void SetNextPosition(int, int);
        virtual void SetPosition(int, int);
        virtual void SetDirection(int);
        virtual void SetState(int);

    public:
        void SetHP(int);

    public:
        virtual void Goto(int, int);
};
