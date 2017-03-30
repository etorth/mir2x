/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 03/29/2017 15:38:36
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
#include "creature.hpp"

class Hero: public Creature
{
    protected:
        uint32_t    m_GUID;
        bool        m_Male;
        int         m_Level;
        int         m_Job;

    public:
        Hero(uint32_t, uint32_t, bool, ProcessRun *);
        ~Hero();

    public:
        void Draw(int, int);
        void Update();

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

        size_t FrameCount()
        {
            return 1;
        }

    public:
        void ResetLevel(int nLevel)
        {
            m_Level = ((nLevel >= 0) ? nLevel : 0);
        }

        void ResetJob(int nJobID)
        {
            m_Job = nJobID;
        }

    public:
        bool ValidG()
        {
            return true;
        }
};
