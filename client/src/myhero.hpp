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
        uint32_t m_Gold;

    private:
        InvPack m_InvPack;

    private:
        std::deque<ActionNode> m_ActionQueue;

    public:
        MyHero(uint64_t, uint32_t, bool, uint32_t, ProcessRun *, const ActionNode &);

    public:
        ~MyHero() = default;

    public:
        bool Update(double);

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
        bool DecompMove(bool,   // bCheckGround
                bool,           // bCheckCreature
                bool,           // bCheckMove
                int, int,       // srcLoc
                int, int,       // dstLoc
                int *, int *);  // decompLoc

    public:
        bool MoveNextMotion();

    protected:
        bool DecompActionMove();
        bool DecompActionSpell();
        bool DecompActionPickUp();
        bool DecompActionAttack();

    public:
        bool ParseActionQueue();

    public:
        uint32_t GetGold() const
        {
            return m_Gold;
        }

        void SetGold(uint32_t nGold)
        {
            m_Gold = nGold;
        }

    public:
        InvPack &GetInvPack()
        {
            return m_InvPack;
        }

    public:
        bool StayIdle();

    public:
        virtual void PickUp();

    public:
        bool EmplaceAction(const ActionNode &);

    public:
        void ReportAction(const ActionNode &);

    public:
        void PullGold();
};
