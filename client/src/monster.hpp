/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 08/31/2015 08:26:19
 *  Last Modified: 07/10/2017 15:10:22
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
#include "game.hpp"
#include "creature.hpp"
#include "protocoldef.hpp"
#include "clientmessage.hpp"

class Monster: public Creature
{
    protected:
        const uint32_t m_MonsterID;

    public:
       static Monster *Create(uint32_t, uint32_t, ProcessRun *, const ActionNode &);

    protected:
        Monster(uint32_t, uint32_t, ProcessRun *);
       ~Monster() = default;

    public:
        int Type() const
        {
            return CREATURE_MONSTER;
        }

    public:
        bool Draw(int, int);
        bool Update();

    public:
        uint32_t MonsterID() const
        {
            return m_MonsterID;
        }

    protected:
        bool Location(int *, int *);

    public:
        bool ParseNewAction(const ActionNode &, bool);

    public:
        bool MotionValid(const MotionNode &)       const;
        bool ActionValid(const ActionNode &, bool) const;

    public:
        bool CanFocus(int, int);

    protected:
        int GfxMotionID(int) const;
        int GfxID(int, int) const;

    public:
        int MotionFrameCount(int, int) const;

    protected:
        MotionNode MakeMotionWalk(int, int, int, int, int) const;

    public:
        int  MaxStep() const;
        int CurrStep() const;

    public:
        int LookID() const;
};
