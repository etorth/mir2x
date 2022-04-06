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
#include "mathf.hpp"
#include "invpack.hpp"
#include "combatnode.hpp"
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

    private:
        bool m_nextStrike = false;

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

        double getLevelRatio() const
        {
            if(const auto level = getLevel(); level == 0){
                return to_df(getExp()) / SYS_EXP(0);
            }
            else{
                return to_df(getExp() - SYS_SUMEXP(level - 1)) / SYS_EXP(level);
            }
        }

    public:
        double getInventoryRatio() const
        {
            return mathf::bound<double>(to_df(getInvPack().getWeight()) / getCombatNode().load.inventory, 0.0, 1.0);
        }

    public:
        CombatNode getCombatNode() const
        {
            return ::getCombatNode(m_sdWLDesp.wear, {}, UID(), getLevel());
        }

    public:
        /* */ SDBelt &getBelt()       { return m_sdBelt; }
        const SDBelt &getBelt() const { return m_sdBelt; }

        /* */ InvPack &getInvPack()       { return m_invPack; }
        const InvPack &getInvPack() const { return m_invPack; }

        /* */ SDItem &getBelt(size_t i)       { return m_sdBelt.list.at(i); }
        const SDItem &getBelt(size_t i) const { return m_sdBelt.list.at(i); }

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
        void setBelt(SDBelt);
        void setBelt(int, SDItem, bool playSound = true);

    public:
        bool canWear(uint32_t, int) const;

    public:
        void setMagicCastTime(uint32_t);
        int getMagicCoolDownAngle(uint32_t) const;

    public:
        bool getNextStrike() const
        {
            return m_nextStrike;
        }

        void setNextStrike(bool nextStrike)
        {
            m_nextStrike = nextStrike;
        }
};
