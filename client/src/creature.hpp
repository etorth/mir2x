/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/15/2016 01:43:23
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

#include "protocoldef.hpp"

class Creature
{
    protected:
        uint32_t m_UID;
        uint32_t m_AddTime;

        int m_X;
        int m_Y;
        int m_R;

        uint32_t m_MapID;

    protected:
        double m_UpdateTime;
        double m_FrameDelay;

    protected:
        int    m_Frame;
        int    m_Speed;
        int    m_Action;
        int    m_Direction;

    public:
        Creature(uint32_t, uint32_t);
        virtual ~Creature();

    public:
        uint32_t MapID()
        {
            return m_MapID;
        }

        int Speed()
        {
            return m_Speed;
        }

    public:
        void ResetAction(int nAction)
        {
            m_Action = nAction;
        }

        void ResetR(uint32_t nR)
        {
            m_R = nR;
        }

        void ResetSpeed(int nSpeed)
        {
            m_Speed = nSpeed;
        }

        void ResetDirection(int nDirection)
        {
            m_Direction = nDirection;
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
        int UID()
        {
            return m_UID;
        }

        int AddTime()
        {
            return m_AddTime;
        }

    public:
        // update the next possible position based on current state
        void EstimateLocation(int, int *, int *);

    public:
        virtual int Type() = 0;
        virtual size_t FrameCount() = 0;

    public:
        virtual bool ValidG() = 0;
        virtual void Draw(int, int) = 0;
        virtual void Update() = 0;
};
