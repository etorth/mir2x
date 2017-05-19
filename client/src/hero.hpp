/*
 * =====================================================================================
 *
 *       Filename: hero.hpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 05/18/2017 01:19:20
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
        const uint32_t m_DBID;

    protected:
        const bool m_Male;

    protected:
        const uint32_t m_DressID;

    protected:
        bool m_OnHorse;

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
        bool ParseNewNotice(const NoticeNode &, bool);
        bool ParseNewAction(const ActionNode &, bool);

    public:
        int Type()
        {
            return CREATURE_PLAYER;
        }

    public:
        bool     Male   () const { return m_Male   ; }
        uint32_t DBID   () const { return m_DBID   ; }
        uint32_t DressID() const { return m_DressID; }


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
