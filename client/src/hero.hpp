/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 04/25/2017 00:12:58
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

    protected:
        Hero(uint32_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

    public:
       ~Hero() = default;

    public:
        bool Location(int *, int *);

    public:
        bool Draw(int, int);
        bool Update();

    public:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const MotionNode &);

    public:
        virtual int32_t GfxID(int, int, int);

    public:
        bool UpdateMotion();

    public:
        bool UpdateMotionOnGeneralMotion(int);

    public:
        bool ParseNewAction(const ActionNode &, bool);
        bool ParseNewState (const  StateNode &, bool);

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

    public:
        bool Moving();

    public:
        size_t MotionFrameCount();

    public:
        bool ValidG()
        {
            return true;
        }
};
