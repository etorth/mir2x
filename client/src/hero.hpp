/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 04/05/2017 14:29:37
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
        uint32_t m_DBID;

    protected:
        bool m_Male;

    protected:
        uint32_t m_DressID;

    public:
        Hero(uint32_t, uint32_t, bool, uint32_t, ProcessRun *, int, int, int, int, int);
       ~Hero() = default;

    public:
        void Draw(int, int);
        void Update();

    public:
        bool UpdateMotion();

    public:
        bool UpdateMotionOnStand();
        bool UpdateMotionOnWalk();
        bool UpdateMotionOnAttack();
        bool UpdateMotionOnUnderAttack();
        bool UpdateMotionOnDie();

    public:
        bool OnReportState();
        bool OnReportAction(int, int, int, int, int, int);

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

        size_t MotionFrameCount()
        {
            return 1;
        }

    public:
        bool ValidG()
        {
            return true;
        }
};
