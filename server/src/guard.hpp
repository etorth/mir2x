/*
 * =====================================================================================
 *
 *       Filename: guard.hpp
 *        Created: 04/26/2021 02:32:45
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
#include "dbcomid.hpp"
#include "monster.hpp"

class Guard: public Monster
{
    private:
        const int m_standX;
        const int m_standY;
        const int m_standDirection;

    public:
        Guard(uint32_t, ServerMap *, int, int, int);

    protected:
        void jumpBack(std::function<void()>, std::function<void()>);

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        corof::long_jmper::eval_op<bool> coro_jumpBack();

    private:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    protected:
        void onAMAttack(const ActorMsgPack &) override
        {
            // guard won't get any damage
        }

    protected:
        bool canMove()   const override;
        bool canAttack() const override;
};
