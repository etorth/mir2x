/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 08/31/2015 08:26:19
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
#include "uidf.hpp"
#include "client.hpp"
#include "creature.hpp"
#include "clientmsg.hpp"
#include "protocoldef.hpp"

class Monster: public Creature
{
    public:
        static Monster *createMonster(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        Monster(uint64_t, ProcessRun *);

    public:
        ~Monster() = default;

    public:
        int type() const override
        {
            return CREATURE_MONSTER;
        }

    public:
        bool update(double) override;
        bool draw(int, int, int) override;

    public:
        uint32_t monsterID() const
        {
            return uidf::getMonsterID(UID());
        }

    protected:
        std::tuple<int, int> location() const override;

    public:
        bool ParseAction(const ActionNode &);

    public:
        bool motionValid(const MotionNode &) const;

    public:
        bool canFocus(int, int) const override;

    protected:
        int GfxMotionID(int) const;
        int GfxID(int, int) const;

    public:
        int motionFrameCount(int, int) const override;

    protected:
        void DispatchSpaceMove();

    protected:
        MotionNode MakeMotionWalk(int, int, int, int, int) const;

    public:
        int  MaxStep() const;
        int CurrStep() const;

    public:
        int LookID() const;
};
