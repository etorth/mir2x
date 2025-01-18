#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerGuard: public Monster
{
    private:
        const int m_standX;
        const int m_standY;
        const int m_standDirection;

    public:
        ServerGuard(uint32_t, ServerMap *, int, int, int);

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    private:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    protected:
        void onAMAttack(const ActorMsgPack &) override
        {
            // serverguard won't get any damage
        }

    protected:
        bool canMove()   const override;
        bool canAttack() const override;
};
