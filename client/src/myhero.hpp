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
        uint32_t m_gold;

    private:
        InvPack m_invPack;

    private:
        std::deque<ActionNode> m_actionQueue;

    public:
        MyHero(uint64_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

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
        bool decompActionPickUp();
        bool decompActionAttack();

    public:
        bool parseActionQueue();

    public:
        uint32_t GetGold() const
        {
            return m_gold;
        }

        void setGold(uint32_t nGold)
        {
            m_gold = nGold;
        }

    public:
        InvPack &getInvPack()
        {
            return m_invPack;
        }

    public:
        bool stayIdle() const override
        {
            return Hero::stayIdle() && m_actionQueue.empty();
        }

    public:
        void pickUp() override;

    public:
        bool emplaceAction(const ActionNode &);

    public:
        void reportAction(const ActionNode &);

    public:
        void pullGold();

    public:
        void flushMotionPending() override;
};
