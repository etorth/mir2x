/*
 * =====================================================================================
 *
 *       Filename: myhero.hpp
 *        Created: 04/07/2016 03:48:41 AM
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
#include "hero.hpp"
#include "invpack.hpp"
#include "actionnode.hpp"

class MyHero: public Hero
{
    private:
        uint32_t m_exp  = 0;
        uint32_t m_gold = 0;

    private:
        std::unordered_map<uint32_t, uint64_t> m_lastCastTime;

    private:
        SDBelt m_sdBelt;
        InvPack m_invPack;

    private:
        std::deque<ActionNode> m_actionQueue;

    public:
        MyHero(uint64_t, ProcessRun *, const ActionNode &);

    public:
        ~MyHero() = default;

    public:
        // decompose (srcLoc->dstLoc) => (srcLoc->decompLoc->dstLoc)
        // this function is used for parsing ACTION_MOVE and ACTION_ATTACK
        // return true if under setting (bCheckGround, bCheckCreature, bCheckMove) we get
        // 0. srcLoc->dstLoc is possible
        // 1. decompLoc != srcLoc
        // 2. srcLoc->decompLoc is one-hop reachable

        // notice:
        // 1. we could have decompLoc == dstLoc
        //    which means provided srcLoc->dstLoc is unit motion
        // 2.

        // parameter: bCheckGround    : see ClientPathFinder
        //          : bCheckCreature  : see ClientPathFinder
        //          : bCheckMove      : true  : srcLoc->decompLoc is valid and non-occupied
        //                            : false : not guaranteed
        bool decompMove(bool,   // bCheckGround
                int,            // nCheckCreature
                bool,           // bCheckMove
                int, int,       // srcLoc
                int, int,       // dstLoc
                int *, int *);  // decompLoc

    public:
        bool moveNextMotion() override;

    protected:
        bool decompActionMove();
        bool decompActionSpell();
        bool decompActionAttack();

    public:
        bool parseActionQueue();
        void clearActionQueue();

    public:
        uint32_t getExp() const
        {
            return m_exp;
        }

        void setExp(uint32_t exp)
        {
            m_exp = exp;
        }

        uint32_t getGold() const
        {
            return m_gold;
        }

        void setGold(uint32_t nGold)
        {
            m_gold = nGold;
        }

    public:
        uint32_t getLevel() const
        {
            return SYS_LEVEL(getExp());
        }

        float getLevelRatio() const
        {
            const auto level = getLevel();
            return to_f(to_df(getExp() - SYS_SUMEXP(level)) / SYS_EXP[level]);
        }

    public:
        InvPack &getInvPack()
        {
            return m_invPack;
        }

        SDBelt &getBelt()
        {
            return m_sdBelt;
        }

        SDItem &getBelt(size_t i)
        {
            return m_sdBelt.list.at(i);
        }

    public:
        bool stayIdle() const override
        {
            return Hero::stayIdle() && m_actionQueue.empty();
        }

    public:
        void brakeMove();

    public:
        bool emplaceAction(const ActionNode &);

    public:
        void reportAction(const ActionNode &);

    public:
        void pullGold();

    public:
        void flushForcedMotion() override;

    public:
        void setBelt(SDBelt belt)
        {
            m_sdBelt = std::move(belt);
        }

        void setBelt(int slot, SDItem item)
        {
            m_sdBelt.list.at(slot) = std::move(item);
        }

    public:
        bool canWear(uint32_t, int) const;

    public:
        void setMagicCastTime(uint32_t);
        int getMagicCoolDownAngle(uint32_t) const;
};
